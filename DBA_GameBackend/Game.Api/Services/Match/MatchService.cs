/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Application.Characters;
using Game.Shared.Contracts.Match;
using Game.Shared.Contracts.Character;
using Game.Shared.Errors;
using Game.Shared.Options;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Infrastructure.Redis;
using Game.ServerManagement.DedicatedServers;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Options;
using StackExchange.Redis;
using System.Security.Cryptography;
using System.Text;

namespace Game.Api.Services.Match;

public interface IMatchService
{
    Task<MatchmakingTicketResponse> CreateTicketAsync(Guid playerId, CreateMatchmakingTicketRequest request);
    Task<MatchmakingTicketResponse?> GetTicketAsync(Guid ticketId);
    Task<bool> CancelTicketAsync(Guid ticketId, Guid playerId);
    Task ProcessMatchmakingTickAsync(CancellationToken cancellationToken = default);
}

public sealed class MatchService : IMatchService
{
    private readonly GameDbContext _db;
    private readonly IRedisConnectionFactory _redis;
    private readonly IDedicatedServerOrchestrator _orchestrator;
    private readonly DedicatedServerOrchestrationOptions _orchestrationOptions;
    private readonly ILogger<MatchService> _logger;
    private readonly ICharacterBuildPolicy _characterBuildPolicy;
    private const int TicketTimeoutSeconds = 300;
    private const string DefaultMapId = "LobbyMap";
    private const int DefaultMmr = 1000;

    public MatchService(
        GameDbContext db,
        IRedisConnectionFactory redis,
        IDedicatedServerOrchestrator orchestrator,
        IOptions<DedicatedServerOrchestrationOptions> orchestrationOptions,
        ILogger<MatchService> logger,
        ICharacterBuildPolicy characterBuildPolicy)
    {
        _db = db;
        _redis = redis;
        _orchestrator = orchestrator;
        _orchestrationOptions = orchestrationOptions.Value;
        _logger = logger;
        _characterBuildPolicy = characterBuildPolicy;
    }

    public async Task<MatchmakingTicketResponse> CreateTicketAsync(Guid playerId, CreateMatchmakingTicketRequest request)
    {
        var existingTicket = await _db.MatchmakingTickets
            .FirstOrDefaultAsync(x => x.PlayerId == playerId && x.Status == "QUEUED");

        if (existingTicket != null)
            throw new InvalidOperationException("Player already has an active matchmaking ticket");

        var mmr = request.Mmr <= 0 ? DefaultMmr : request.Mmr;
        var ticket = new MatchmakingTicket
        {
            Id = Guid.NewGuid(),
            PlayerId = playerId,
            Mode = request.Mode,
            Region = request.Region,
            Mmr = mmr,
            Status = "QUEUED",
            TimeoutAt = DateTimeOffset.UtcNow.AddSeconds(TicketTimeoutSeconds),
            CreatedAt = DateTimeOffset.UtcNow
        };

        _db.MatchmakingTickets.Add(ticket);
        await _db.SaveChangesAsync();

        try
        {
            var dbRedis = _redis.GetDatabase();
            await dbRedis.SortedSetAddAsync($"matchmaking:{request.Mode}:{request.Region}", ticket.Id.ToString(), mmr);
        }
        catch (Exception ex)
        {
            // Redis 仅作加速队列；DB 为权威来源，失败不阻断入队。
            _logger.LogWarning(ex, "匹配票据 {TicketId} 写入 Redis 队列失败，将依赖数据库配对", ticket.Id);
        }

        // 创建后立即尝试配对，降低双客户端联调等待。
        await ProcessMatchmakingTickAsync();

        var refreshed = await _db.MatchmakingTickets.FindAsync(ticket.Id);
        return ToResponse(refreshed ?? ticket);
    }

    public async Task<MatchmakingTicketResponse?> GetTicketAsync(Guid ticketId)
    {
        // 轮询时顺带推进配对，避免仅依赖后台周期。
        await ProcessMatchmakingTickAsync();

        var ticket = await _db.MatchmakingTickets.FindAsync(ticketId);
        return ticket == null ? null : ToResponse(ticket);
    }

    public async Task<bool> CancelTicketAsync(Guid ticketId, Guid playerId)
    {
        var ticket = await _db.MatchmakingTickets
            .FirstOrDefaultAsync(x => x.Id == ticketId && x.PlayerId == playerId && x.Status == "QUEUED");

        if (ticket == null) return false;

        ticket.Status = "CANCELLED";
        ticket.CancelledAt = DateTimeOffset.UtcNow;
        await _db.SaveChangesAsync();

        try
        {
            var dbRedis = _redis.GetDatabase();
            await dbRedis.SortedSetRemoveAsync($"matchmaking:{ticket.Mode}:{ticket.Region}", ticket.Id.ToString());
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "匹配票据 {TicketId} 从 Redis 队列移除失败", ticket.Id);
        }

        return true;
    }

    public async Task ProcessMatchmakingTickAsync(CancellationToken cancellationToken = default)
    {
        var now = DateTimeOffset.UtcNow;
        var timeoutTickets = await _db.MatchmakingTickets
            .Where(x => x.Status == "QUEUED" && x.TimeoutAt <= now)
            .ToListAsync(cancellationToken);

        foreach (var ticket in timeoutTickets)
        {
            ticket.Status = "TIMEOUT";
            ticket.UpdatedAt = now;
            _logger.LogInformation("匹配票据 {TicketId} 已超时", ticket.Id);
            await TryRemoveFromRedisAsync(ticket);
        }

        if (timeoutTickets.Count > 0)
        {
            await _db.SaveChangesAsync(cancellationToken);
        }

        var buckets = await _db.MatchmakingTickets
            .AsNoTracking()
            .Where(x => x.Status == "QUEUED")
            .Select(x => new { x.Mode, x.Region })
            .Distinct()
            .ToListAsync(cancellationToken);

        foreach (var bucket in buckets)
        {
            await TryPairBucketAsync(bucket.Mode, bucket.Region, cancellationToken);
        }
    }

    private async Task TryPairBucketAsync(string mode, string region, CancellationToken cancellationToken)
    {
        while (!cancellationToken.IsCancellationRequested)
        {
            var candidates = await _db.MatchmakingTickets
                .Where(x => x.Status == "QUEUED" && x.Mode == mode && x.Region == region)
                .OrderBy(x => x.CreatedAt)
                .Take(2)
                .ToListAsync(cancellationToken);

            if (candidates.Count < 2)
            {
                return;
            }

            var ticketA = candidates[0];
            var ticketB = candidates[1];
            var session = await CreateMatchedSessionAsync(ticketA, ticketB, cancellationToken);
            if (session is null)
            {
                return;
            }

            await TryRemoveFromRedisAsync(ticketA);
            await TryRemoveFromRedisAsync(ticketB);
            await TryAllocateServerForMatchedSessionAsync(session, cancellationToken);
        }
    }

    private async Task TryAllocateServerForMatchedSessionAsync(GameSession session, CancellationToken cancellationToken)
    {
        // External / Mock 模式下由外部编排脚本分配并拉起 DS，避免 mock 分配后丢失 runtimeToken。
        var serverMode = _orchestrationOptions.ServerMode ?? string.Empty;
        if (serverMode.Equals("External", StringComparison.OrdinalIgnoreCase)
            || _orchestrationOptions.AllowMockServerAllocation)
        {
            _logger.LogInformation(
                "匹配会话 {SessionId} 已创建；当前编排模式={Mode} AllowMock={AllowMock}，等待外部分配 Dedicated Server",
                session.Id,
                serverMode,
                _orchestrationOptions.AllowMockServerAllocation);
            return;
        }

        try
        {
            var allocated = await _orchestrator.AllocateAsync(
                new AllocateDedicatedServerCommand(
                    session.Id,
                    session.Mode,
                    string.IsNullOrWhiteSpace(session.MapId) ? DefaultMapId : session.MapId,
                    session.Region,
                    "local-matchmaking"),
                cancellationToken);

            if (allocated is null)
            {
                _logger.LogWarning(
                    "匹配会话 {SessionId} 已创建，但 Dedicated Server 分配失败（可能达到上限或端口耗尽）",
                    session.Id);
            }
            else
            {
                _logger.LogInformation(
                    "匹配会话 {SessionId} 已分配服务器 {ServerId} {Ip}:{Port}",
                    session.Id,
                    allocated.ServerId,
                    allocated.PublicIp,
                    allocated.Port);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "匹配会话 {SessionId} 分配 Dedicated Server 时异常", session.Id);
        }
    }

    private async Task<GameSession?> CreateMatchedSessionAsync(
        MatchmakingTicket ticketA,
        MatchmakingTicket ticketB,
        CancellationToken cancellationToken)
    {
        await using var tx = await _db.Database.BeginTransactionAsync(cancellationToken);
        try
        {
            // 重新加载并校验，避免并发 tick 重复配对。
            var freshA = await _db.MatchmakingTickets.FindAsync([ticketA.Id], cancellationToken);
            var freshB = await _db.MatchmakingTickets.FindAsync([ticketB.Id], cancellationToken);
            if (freshA is null || freshB is null
                || freshA.Status != "QUEUED" || freshB.Status != "QUEUED"
                || !string.Equals(freshA.Mode, freshB.Mode, StringComparison.Ordinal)
                || !string.Equals(freshA.Region, freshB.Region, StringComparison.Ordinal))
            {
                await tx.RollbackAsync(cancellationToken);
                return null;
            }

            var now = DateTimeOffset.UtcNow;
            var session = new GameSession
            {
                Id = Guid.NewGuid(),
                SourceType = "MATCH",
                SourceId = freshA.Id,
                Mode = freshA.Mode,
                MapId = DefaultMapId,
                Region = freshA.Region,
                Status = "CREATED",
                MaxPlayers = 2,
                RetryCount = 0,
                CreatedAt = now
            };

            freshA.Status = "MATCHED";
            freshA.MatchedSessionId = session.Id;
            freshA.UpdatedAt = now;
            freshB.Status = "MATCHED";
            freshB.MatchedSessionId = session.Id;
            freshB.UpdatedAt = now;

            _db.GameSessions.Add(session);

            var selectedCharacters = await LoadSelectedCharactersByPlayerAsync(
                new[] { freshA.PlayerId, freshB.PlayerId },
                cancellationToken);

            AddPlayerSession(session.Id, freshA.PlayerId, team: "1", slotIndex: 0, selectedCharacters, now);
            AddPlayerSession(session.Id, freshB.PlayerId, team: "2", slotIndex: 1, selectedCharacters, now);

            _db.SessionEvents.Add(new SessionEvent
            {
                Id = Guid.NewGuid(),
                GameSessionId = session.Id,
                EventType = "MATCH_CREATED",
                PayloadJson = $$"""{"ticketA":"{{freshA.Id}}","ticketB":"{{freshB.Id}}"}""",
                CreatedAt = now
            });

            await _db.SaveChangesAsync(cancellationToken);
            await tx.CommitAsync(cancellationToken);

            _logger.LogInformation(
                "匹配成功：会话 {SessionId}，票据 {TicketA} + {TicketB}",
                session.Id,
                freshA.Id,
                freshB.Id);

            return session;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "创建匹配会话失败：票据 {TicketA}/{TicketB}", ticketA.Id, ticketB.Id);
            try { await tx.RollbackAsync(cancellationToken); } catch { /* ignore */ }
            return null;
        }
    }

    private void AddPlayerSession(
        Guid sessionId,
        Guid playerId,
        string team,
        int slotIndex,
        IReadOnlyDictionary<Guid, PlayerCharacter> selectedCharacters,
        DateTimeOffset now)
    {
        selectedCharacters.TryGetValue(playerId, out var selectedCharacter);
        var buildSummary = selectedCharacter is null
            ? null
            : _characterBuildPolicy.BuildSummary(
                selectedCharacter.Zodiac,
                selectedCharacter.PrimaryElement,
                selectedCharacter.FiveCamp);

        var token = Guid.NewGuid().ToString();
        _db.PlayerSessions.Add(new PlayerSession
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            PlayerId = playerId,
            Team = team,
            SlotIndex = slotIndex,
            Zodiac = buildSummary?.Zodiac,
            PrimaryElement = buildSummary?.PrimaryElement,
            FiveCamp = buildSummary?.FiveCamp,
            FixedSkillGroupId = buildSummary?.FixedSkillGroupId,
            Status = "CREATED",
            SessionTokenHash = HashToken(token),
            SessionTokenExpiresAt = now.AddHours(2),
            CreatedAt = now
        });
    }

    private async Task<Dictionary<Guid, PlayerCharacter>> LoadSelectedCharactersByPlayerAsync(
        IEnumerable<Guid> playerIds,
        CancellationToken cancellationToken)
    {
        var ids = playerIds.Distinct().ToList();
        var selectedCharacters = await _db.PlayerCharacters
            .Where(x => ids.Contains(x.PlayerId) && x.IsSelected)
            .OrderByDescending(x => x.LastUsedAt)
            .ToListAsync(cancellationToken);

        return selectedCharacters
            .GroupBy(x => x.PlayerId)
            .ToDictionary(x => x.Key, x => x.First());
    }

    private async Task TryRemoveFromRedisAsync(MatchmakingTicket ticket)
    {
        try
        {
            var dbRedis = _redis.GetDatabase();
            await dbRedis.SortedSetRemoveAsync($"matchmaking:{ticket.Mode}:{ticket.Region}", ticket.Id.ToString());
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "匹配票据 {TicketId} 从 Redis 队列移除失败", ticket.Id);
        }
    }

    private static string HashToken(string token)
    {
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(token));
        return Convert.ToHexString(hash).ToLowerInvariant();
    }

    private static MatchmakingTicketResponse ToResponse(MatchmakingTicket t) =>
        new(t.Id, t.PlayerId, t.Mode, t.Region, t.Mmr, t.Status, t.MatchedSessionId, t.CreatedAt, t.TimeoutAt);
}
