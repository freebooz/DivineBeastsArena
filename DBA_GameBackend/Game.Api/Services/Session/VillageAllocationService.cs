/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：把已认证且已选择角色的玩家分配到共享新手村会话，并触发 Dedicated Server 编排。
- 安全约束：这里只建立玩家与会话关系；明文 JoinTicket 由连接信息接口按次签发。
*/

using System.Security.Cryptography;
using System.Text;
using Game.Application.Characters;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.ServerManagement.DedicatedServers;
using Game.Shared.Contracts.Character;
using Game.Shared.Contracts.Session;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Options;

namespace Game.Api.Services.Session;

public interface IVillageAllocationService
{
    Task<VillageAllocationResponse?> AllocateAsync(
        Guid playerId,
        Guid characterId,
        CancellationToken cancellationToken = default);
}

public sealed class VillageAllocationService : IVillageAllocationService
{
    private static readonly string[] JoinableSessionStatuses =
    {
        "CREATED",
        "ALLOCATING_SERVER",
        "WAITING_PLAYERS",
        "IN_PROGRESS"
    };
    private static readonly string[] JoinableServerStatuses =
    {
        "STARTING",
        "READY",
        "ALLOCATED",
        "IN_GAME",
        "IN_PROGRESS"
    };

    private readonly GameDbContext _db;
    private readonly IDedicatedServerOrchestrator _orchestrator;
    private readonly VillageSessionOptions _options;
    private readonly ILogger<VillageAllocationService> _logger;
    private readonly ICharacterBuildPolicy _characterBuildPolicy;
    private readonly DedicatedServerOrchestrationOptions _serverOptions;

    public VillageAllocationService(
        GameDbContext db,
        IDedicatedServerOrchestrator orchestrator,
        IOptions<VillageSessionOptions> options,
        IOptions<DedicatedServerOrchestrationOptions> serverOptions,
        ILogger<VillageAllocationService> logger,
        ICharacterBuildPolicy characterBuildPolicy)
    {
        _db = db;
        _orchestrator = orchestrator;
        _options = options.Value;
        _serverOptions = serverOptions.Value;
        _logger = logger;
        _characterBuildPolicy = characterBuildPolicy;
    }

    public async Task<VillageAllocationResponse?> AllocateAsync(
        Guid playerId,
        Guid characterId,
        CancellationToken cancellationToken = default)
    {
        var heartbeatCutoff = DateTimeOffset.UtcNow.AddSeconds(
            -Math.Max(30, _serverOptions.HeartbeatTimeoutSeconds));
        var joinableServerIds = BuildJoinableServerIds(heartbeatCutoff);
        var selectedCharacter = await _db.PlayerCharacters
            .FirstOrDefaultAsync(
                x => x.Id == characterId && x.PlayerId == playerId && x.IsSelected,
                cancellationToken);
        if (selectedCharacter is null)
        {
            _logger.LogWarning("新手村分配被拒绝：玩家没有有效的已选角色。玩家={PlayerId} 角色={CharacterId}", playerId, characterId);
            return null;
        }

        var existingPlayerSession = await _db.PlayerSessions
            .Include(x => x.GameSession)
            .Where(x => x.PlayerId == playerId
                && x.LeftAt == null
                && x.GameSession != null
                && x.GameSession.SourceType == "VILLAGE"
                && JoinableSessionStatuses.Contains(x.GameSession.Status)
                && ((x.GameSession.Status == "CREATED" && x.GameSession.ServerId == null)
                    || (x.GameSession.ServerId != null
                        && joinableServerIds.Contains(x.GameSession.ServerId.Value))))
            .OrderByDescending(x => x.CreatedAt)
            .FirstOrDefaultAsync(cancellationToken);

        GameSession session;
        if (existingPlayerSession?.GameSession is not null)
        {
            session = existingPlayerSession.GameSession;
            ApplySelectedCharacter(existingPlayerSession, selectedCharacter);
        }
        else
        {
            session = await FindJoinableVillageAsync(
                joinableServerIds,
                cancellationToken) ?? CreateVillageSession();
            if (_db.Entry(session).State == EntityState.Detached)
            {
                _db.GameSessions.Add(session);
            }

            var placeholderToken = Convert.ToBase64String(RandomNumberGenerator.GetBytes(32));
            var playerSession = new PlayerSession
            {
                Id = Guid.NewGuid(),
                GameSessionId = session.Id,
                PlayerId = playerId,
                SlotIndex = await ResolveNextSlotIndexAsync(session.Id, cancellationToken),
                Team = "village",
                Status = "CREATED",
                SessionTokenHash = HashToken(placeholderToken),
                SessionTokenExpiresAt = DateTimeOffset.UtcNow,
                CreatedAt = DateTimeOffset.UtcNow
            };
            ApplySelectedCharacter(playerSession, selectedCharacter);
            _db.PlayerSessions.Add(playerSession);
        }

        session.UpdatedAt = DateTimeOffset.UtcNow;
        await _db.SaveChangesAsync(cancellationToken);

        if (session.ServerId is null)
        {
            var allocated = await _orchestrator.AllocateAsync(
                new AllocateDedicatedServerCommand(
                    session.Id,
                    session.Mode,
                    session.MapId,
                    session.Region,
                    session.BuildVersion),
                cancellationToken);
            if (allocated is null)
            {
                _logger.LogError("新手村服务器分配失败。会话={SessionId}", session.Id);
                return null;
            }
        }

        await _db.Entry(session).ReloadAsync(cancellationToken);
        _logger.LogInformation("新手村分配完成。玩家={PlayerId} 会话={SessionId} 状态={Status}", playerId, session.Id, session.Status);
        return new VillageAllocationResponse(session.Id, session.Status);
    }

    private async Task<GameSession?> FindJoinableVillageAsync(
        IQueryable<Guid> joinableServerIds,
        CancellationToken cancellationToken)
    {
        var maxPlayers = Math.Clamp(_options.MaxPlayers, 1, 64);
        return await _db.GameSessions
            .Include(x => x.PlayerSessions)
            .Where(x => x.SourceType == "VILLAGE"
                && x.Mode == _options.Mode
                && x.MapId == _options.MapId
                && x.Region == _options.Region
                && JoinableSessionStatuses.Contains(x.Status)
                && ((x.Status == "CREATED" && x.ServerId == null)
                    || (x.ServerId != null && joinableServerIds.Contains(x.ServerId.Value)))
                && x.PlayerSessions.Count(p => p.LeftAt == null) < maxPlayers)
            .OrderBy(x => x.CreatedAt)
            .FirstOrDefaultAsync(cancellationToken);
    }

    private IQueryable<Guid> BuildJoinableServerIds(DateTimeOffset heartbeatCutoff)
    {
        return _db.GameServerInstances
            .Where(server =>
                JoinableServerStatuses.Contains(server.Status)
                && server.LastHeartbeatAt != null
                && server.LastHeartbeatAt >= heartbeatCutoff)
            .Select(server => server.Id);
    }

    private GameSession CreateVillageSession()
    {
        var now = DateTimeOffset.UtcNow;
        return new GameSession
        {
            Id = Guid.NewGuid(),
            SourceType = "VILLAGE",
            Mode = _options.Mode.Trim(),
            MapId = _options.MapId.Trim(),
            Region = _options.Region.Trim(),
            BuildVersion = string.IsNullOrWhiteSpace(_options.BuildVersion) ? null : _options.BuildVersion.Trim(),
            Status = "CREATED",
            MaxPlayers = Math.Clamp(_options.MaxPlayers, 1, 64),
            CreatedAt = now,
            UpdatedAt = now
        };
    }

    private async Task<int> ResolveNextSlotIndexAsync(Guid sessionId, CancellationToken cancellationToken)
    {
        var occupiedSlots = await _db.PlayerSessions
            .Where(x => x.GameSessionId == sessionId && x.LeftAt == null && x.SlotIndex.HasValue)
            .Select(x => x.SlotIndex!.Value)
            .ToListAsync(cancellationToken);
        var occupied = occupiedSlots.ToHashSet();
        for (var slot = 0; slot < Math.Clamp(_options.MaxPlayers, 1, 64); slot++)
        {
            if (!occupied.Contains(slot)) return slot;
        }
        return occupied.Count;
    }

    private void ApplySelectedCharacter(PlayerSession playerSession, PlayerCharacter selectedCharacter)
    {
        var build = _characterBuildPolicy.BuildSummary(
            selectedCharacter.Zodiac,
            selectedCharacter.PrimaryElement,
            selectedCharacter.FiveCamp);
        playerSession.CharacterId = selectedCharacter.Id;
        playerSession.Zodiac = build.Zodiac;
        playerSession.PrimaryElement = build.PrimaryElement;
        playerSession.FiveCamp = build.FiveCamp;
        playerSession.FixedSkillGroupId = build.FixedSkillGroupId;
    }

    private static string HashToken(string token)
    {
        return Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(token))).ToLowerInvariant();
    }
}
