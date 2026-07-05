/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Session;
using Game.Shared.Contracts.Character;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;
using System.Security.Cryptography;
using System.Text;

namespace Game.Api.Services.Session;

public interface ISessionService
{
    Task<SessionResponse?> GetSessionAsync(Guid sessionId);
    Task<SessionConnectionResponse?> GetConnectionInfoAsync(Guid sessionId, Guid playerId);
    Task<SessionResponse?> CreateFromRoomAsync(Guid roomId);
    Task<SessionResponse?> CreateFromMatchAsync(Guid ticketId);
    Task<SessionResponse?> AllocateServerAsync(Guid sessionId, string ip, int port, string runtimeToken);
    Task<SessionResponse?> MarkInProgressAsync(Guid sessionId);
    Task<SessionResponse?> MarkCompletedAsync(Guid sessionId);
    Task<SessionResponse?> MarkFailedAsync(Guid sessionId, string reason);
}

public sealed class SessionService : ISessionService
{
    private readonly GameDbContext _db;
    private readonly ILogger<SessionService> _logger;

    public SessionService(GameDbContext db, ILogger<SessionService> logger)
    {
        _db = db;
        _logger = logger;
    }

    public async Task<SessionResponse?> GetSessionAsync(Guid sessionId)
    {
        var session = await _db.GameSessions.FindAsync(sessionId);
        return session == null ? null : ToResponse(session);
    }

    public async Task<SessionConnectionResponse?> GetConnectionInfoAsync(Guid sessionId, Guid playerId)
    {
        var playerSession = await _db.PlayerSessions
            .FirstOrDefaultAsync(x => x.GameSessionId == sessionId && x.PlayerId == playerId && x.LeftAt == null);

        if (playerSession == null) return null;

        var session = await _db.GameSessions.FindAsync(sessionId);
        if (session == null || session.ServerIp == null || session.ServerPort == null)
            return null;

        if (session.Status is not ("WAITING_PLAYERS" or "IN_PROGRESS"))
            return null;

        if (!await EnsureFrozenBuildSummaryAsync(playerSession))
        {
            return null;
        }

        // 连接令牌是进入 UE Dedicated Server 的短期凭证，只在本次连接请求中返回明文。
        // 数据库仍然只保存 Hash，避免长期保存可直接进服的敏感 token。
        var playerSessionToken = Convert.ToBase64String(RandomNumberGenerator.GetBytes(32));
        playerSession.SessionTokenHash = HashToken(playerSessionToken);
        playerSession.SessionTokenExpiresAt = DateTimeOffset.UtcNow.AddMinutes(10);
        await _db.SaveChangesAsync();

        return new SessionConnectionResponse(
            sessionId, session.ServerIp, session.ServerPort!.Value,
            playerSessionToken,
            playerSession.SessionTokenExpiresAt,
            playerSessionToken,
            playerId,
            ResolveSessionTeamId(playerSession),
            ToCharacterBuildSummary(playerSession));
    }

    public async Task<SessionResponse?> CreateFromRoomAsync(Guid roomId)
    {
        var existing = await _db.GameSessions
            .FirstOrDefaultAsync(x => x.SourceType == "ROOM" && x.SourceId == roomId);
        if (existing != null) return ToResponse(existing);

        var room = await _db.GameRooms.FindAsync(roomId);
        if (room == null) return null;

        var session = new GameSession
        {
            Id = Guid.NewGuid(),
            SourceType = "ROOM",
            SourceId = roomId,
            Mode = room.Mode,
            MapId = room.MapId,
            Region = room.Region,
            Status = "CREATED",
            MaxPlayers = room.MaxPlayers,
            RetryCount = 0,
            CreatedAt = DateTimeOffset.UtcNow
        };

        room.Status = "IN_GAME";
        _db.GameSessions.Add(session);

        var players = await _db.GameRoomPlayers
            .Where(x => x.RoomId == roomId && x.LeftAt == null)
            .ToListAsync();

        var selectedCharacters = await LoadSelectedCharactersByPlayerAsync(players.Select(x => x.PlayerId));

        foreach (var rp in players)
        {
            var token = Guid.NewGuid().ToString();
            var tokenHash = HashToken(token);
            selectedCharacters.TryGetValue(rp.PlayerId, out var selectedCharacter);
            var buildSummary = selectedCharacter is null ? null : ToCharacterBuildSummary(selectedCharacter);

            _db.PlayerSessions.Add(new PlayerSession
            {
                Id = Guid.NewGuid(),
                GameSessionId = session.Id,
                PlayerId = rp.PlayerId,
                SlotIndex = rp.SlotIndex,
                Team = rp.Team,
                Zodiac = buildSummary?.Zodiac,
                PrimaryElement = buildSummary?.PrimaryElement,
                FiveCamp = buildSummary?.FiveCamp,
                FixedSkillGroupId = buildSummary?.FixedSkillGroupId,
                Status = "CREATED",
                SessionTokenHash = tokenHash,
                SessionTokenExpiresAt = DateTimeOffset.UtcNow.AddHours(2),
                CreatedAt = DateTimeOffset.UtcNow
            });
        }

        await _db.SaveChangesAsync();
        return ToResponse(session);
    }

    public async Task<SessionResponse?> CreateFromMatchAsync(Guid ticketId)
    {
        var existing = await _db.GameSessions
            .FirstOrDefaultAsync(x => x.SourceType == "MATCH" && x.SourceId == ticketId);
        if (existing != null) return ToResponse(existing);

        var ticket = await _db.MatchmakingTickets.FindAsync(ticketId);
        if (ticket == null || ticket.Status != "QUEUED") return null;

        var session = new GameSession
        {
            Id = Guid.NewGuid(),
            SourceType = "MATCH",
            SourceId = ticketId,
            Mode = ticket.Mode,
            MapId = "",
            Region = ticket.Region,
            Status = "CREATED",
            MaxPlayers = 2,
            RetryCount = 0,
            CreatedAt = DateTimeOffset.UtcNow
        };

        ticket.Status = "MATCHED";
        ticket.MatchedSessionId = session.Id;
        _db.GameSessions.Add(session);

        var token = Guid.NewGuid().ToString();
        var tokenHash = HashToken(token);
        var selectedCharacter = await LoadSelectedCharacterAsync(ticket.PlayerId);
        var buildSummary = selectedCharacter is null ? null : ToCharacterBuildSummary(selectedCharacter);

        _db.PlayerSessions.Add(new PlayerSession
        {
            Id = Guid.NewGuid(),
            GameSessionId = session.Id,
            PlayerId = ticket.PlayerId,
            Zodiac = buildSummary?.Zodiac,
            PrimaryElement = buildSummary?.PrimaryElement,
            FiveCamp = buildSummary?.FiveCamp,
            FixedSkillGroupId = buildSummary?.FixedSkillGroupId,
            Status = "CREATED",
            SessionTokenHash = tokenHash,
            SessionTokenExpiresAt = DateTimeOffset.UtcNow.AddHours(2),
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        return ToResponse(session);
    }

    public async Task<SessionResponse?> AllocateServerAsync(Guid sessionId, string ip, int port, string runtimeToken)
    {
        var session = await _db.GameSessions.FindAsync(sessionId);
        if (session == null || session.Status != "CREATED") return null;

        session.ServerId = Guid.NewGuid();
        session.ServerIp = ip;
        session.ServerPort = port;
        session.Status = "ALLOCATING_SERVER";
        session.AllocatedAt = DateTimeOffset.UtcNow;

        await _db.SaveChangesAsync();
        return ToResponse(session);
    }

    public async Task<SessionResponse?> MarkInProgressAsync(Guid sessionId)
    {
        var session = await _db.GameSessions.FindAsync(sessionId);
        if (session == null || session.Status != "WAITING_PLAYERS") return null;

        session.Status = "IN_PROGRESS";
        session.StartedAt = DateTimeOffset.UtcNow;

        _db.SessionEvents.Add(new SessionEvent
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            EventType = "STARTED",
            PayloadJson = "{}",
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        return ToResponse(session);
    }

    public async Task<SessionResponse?> MarkCompletedAsync(Guid sessionId)
    {
        var session = await _db.GameSessions.FindAsync(sessionId);
        if (session == null || session.Status != "IN_PROGRESS" && session.Status != "SETTLING") return null;

        session.Status = "COMPLETED";
        session.EndedAt = DateTimeOffset.UtcNow;

        _db.SessionEvents.Add(new SessionEvent
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            EventType = "COMPLETED",
            PayloadJson = "{}",
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        return ToResponse(session);
    }

    public async Task<SessionResponse?> MarkFailedAsync(Guid sessionId, string reason)
    {
        var session = await _db.GameSessions.FindAsync(sessionId);
        if (session == null) return null;

        session.Status = "FAILED";
        session.EndedAt = DateTimeOffset.UtcNow;

        _db.SessionEvents.Add(new SessionEvent
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            EventType = "FAILED",
            PayloadJson = $"{{\"reason\":\"{reason}\"}}",
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        return ToResponse(session);
    }

    private static SessionResponse ToResponse(GameSession s) => new(
        s.Id, s.SourceType, s.Mode, s.MapId, s.Region, s.Status,
        s.ServerIp, s.ServerPort, s.MaxPlayers, s.CreatedAt, s.StartedAt);

    private async Task<bool> EnsureFrozenBuildSummaryAsync(PlayerSession playerSession)
    {
        var bHasFrozenZodiac = !string.IsNullOrWhiteSpace(playerSession.Zodiac);
        var bHasFrozenPrimaryElement = !string.IsNullOrWhiteSpace(playerSession.PrimaryElement);
        var bHasFrozenFixedSkillGroupId = !string.IsNullOrWhiteSpace(playerSession.FixedSkillGroupId);
        var bHasAnyFrozenBuildSummary = bHasFrozenZodiac || bHasFrozenPrimaryElement || bHasFrozenFixedSkillGroupId;
        var bHasCompleteFrozenBuildSummary = bHasFrozenZodiac && bHasFrozenPrimaryElement && bHasFrozenFixedSkillGroupId;

        if (bHasAnyFrozenBuildSummary && !bHasCompleteFrozenBuildSummary)
        {
            return false;
        }

        if (bHasCompleteFrozenBuildSummary)
        {
            var existingZodiac = CharacterBuildRules.NormalizeChoice(playerSession.Zodiac, "Rat");
            var existingPrimaryElement = CharacterBuildRules.NormalizeChoice(playerSession.PrimaryElement, "Water");
            var existingFixedSkillGroupId = CharacterBuildRules.NormalizeChoice(
                playerSession.FixedSkillGroupId,
                CharacterBuildRules.BuildFixedSkillGroupId(existingZodiac, existingPrimaryElement));
            var expectedFixedSkillGroupId = CharacterBuildRules.BuildFixedSkillGroupId(existingZodiac, existingPrimaryElement);

            if (!string.Equals(existingFixedSkillGroupId, expectedFixedSkillGroupId, StringComparison.OrdinalIgnoreCase))
            {
                return false;
            }

            var existingBuildSummary = CharacterBuildRules.BuildSummary(
                existingZodiac,
                existingPrimaryElement,
                playerSession.FiveCamp);
            playerSession.Zodiac = existingBuildSummary.Zodiac;
            playerSession.PrimaryElement = existingBuildSummary.PrimaryElement;
            playerSession.FiveCamp = existingBuildSummary.FiveCamp;
            playerSession.FixedSkillGroupId = existingBuildSummary.FixedSkillGroupId;
            return true;
        }

        var selectedCharacter = await LoadSelectedCharacterAsync(playerSession.PlayerId);
        if (selectedCharacter is null)
        {
            return true;
        }

        var buildSummary = ToCharacterBuildSummary(selectedCharacter);
        playerSession.Zodiac = buildSummary.Zodiac;
        playerSession.PrimaryElement = buildSummary.PrimaryElement;
        playerSession.FiveCamp = buildSummary.FiveCamp;
        playerSession.FixedSkillGroupId = buildSummary.FixedSkillGroupId;
        return true;
    }

    private async Task<Dictionary<Guid, PlayerCharacter>> LoadSelectedCharactersByPlayerAsync(IEnumerable<Guid> playerIds)
    {
        var ids = playerIds.Distinct().ToList();
        var selectedCharacters = await _db.PlayerCharacters
            .Where(x => ids.Contains(x.PlayerId) && x.IsSelected)
            .OrderByDescending(x => x.LastUsedAt)
            .ToListAsync();

        return selectedCharacters
            .GroupBy(x => x.PlayerId)
            .ToDictionary(x => x.Key, x => x.First());
    }

    private async Task<PlayerCharacter?> LoadSelectedCharacterAsync(Guid playerId)
    {
        return await _db.PlayerCharacters
            .Where(x => x.PlayerId == playerId && x.IsSelected)
            .OrderByDescending(x => x.LastUsedAt)
            .FirstOrDefaultAsync();
    }

    private static CharacterBuildSummaryDto? ToCharacterBuildSummary(PlayerSession playerSession)
    {
        if (string.IsNullOrWhiteSpace(playerSession.Zodiac)
            || string.IsNullOrWhiteSpace(playerSession.PrimaryElement)
            || string.IsNullOrWhiteSpace(playerSession.FixedSkillGroupId))
        {
            return null;
        }

        return CharacterBuildRules.BuildSummary(
            playerSession.Zodiac,
            playerSession.PrimaryElement,
            playerSession.FiveCamp);
    }

    private static CharacterBuildSummaryDto ToCharacterBuildSummary(PlayerCharacter selectedCharacter)
    {
        return CharacterBuildRules.BuildSummary(
            selectedCharacter.Zodiac,
            selectedCharacter.PrimaryElement,
            selectedCharacter.FiveCamp);
    }

    private static string HashToken(string token)
    {
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(token));
        return Convert.ToHexString(hash).ToLowerInvariant();
    }

    private static int ResolveSessionTeamId(PlayerSession playerSession)
    {
        var team = playerSession.Team?.Trim();
        if (!string.IsNullOrWhiteSpace(team))
        {
            if (int.TryParse(team, out var numericTeam) && numericTeam > 0)
            {
                return numericTeam;
            }

            if (team.Equals("blue", StringComparison.OrdinalIgnoreCase) ||
                team.Equals("team1", StringComparison.OrdinalIgnoreCase))
            {
                return 1;
            }

            if (team.Equals("red", StringComparison.OrdinalIgnoreCase) ||
                team.Equals("team2", StringComparison.OrdinalIgnoreCase))
            {
                return 2;
            }
        }

        return playerSession.SlotIndex.HasValue ? playerSession.SlotIndex.Value % 2 + 1 : 0;
    }
}
