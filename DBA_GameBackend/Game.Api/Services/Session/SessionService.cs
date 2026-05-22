/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Session;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

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

        return new SessionConnectionResponse(
            sessionId, session.ServerIp, session.ServerPort!.Value,
            "", // sessionToken hidden for security
            playerSession.SessionTokenExpiresAt);
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

        foreach (var rp in players)
        {
            var token = Guid.NewGuid().ToString();
            var tokenHash = Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(
                System.Text.Encoding.UTF8.GetBytes(token))).ToLowerInvariant();

            _db.PlayerSessions.Add(new PlayerSession
            {
                Id = Guid.NewGuid(),
                GameSessionId = session.Id,
                PlayerId = rp.PlayerId,
                SlotIndex = rp.SlotIndex,
                Team = rp.Team,
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
        var tokenHash = Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(
            System.Text.Encoding.UTF8.GetBytes(token))).ToLowerInvariant();

        _db.PlayerSessions.Add(new PlayerSession
        {
            Id = Guid.NewGuid(),
            GameSessionId = session.Id,
            PlayerId = ticket.PlayerId,
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
}
