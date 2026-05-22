/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Match;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Infrastructure.Redis;
using Microsoft.EntityFrameworkCore;
using StackExchange.Redis;

namespace Game.Api.Services.Match;

public interface IMatchService
{
    Task<MatchmakingTicketResponse> CreateTicketAsync(Guid playerId, CreateMatchmakingTicketRequest request);
    Task<MatchmakingTicketResponse?> GetTicketAsync(Guid ticketId);
    Task<bool> CancelTicketAsync(Guid ticketId, Guid playerId);
    Task ProcessMatchmakingTickAsync();
}

public sealed class MatchService : IMatchService
{
    private readonly GameDbContext _db;
    private readonly IRedisConnectionFactory _redis;
    private readonly ILogger<MatchService> _logger;
    private const int TicketTimeoutSeconds = 300;

    public MatchService(GameDbContext db, IRedisConnectionFactory redis, ILogger<MatchService> logger)
    {
        _db = db;
        _redis = redis;
        _logger = logger;
    }

    public async Task<MatchmakingTicketResponse> CreateTicketAsync(Guid playerId, CreateMatchmakingTicketRequest request)
    {
        var existingTicket = await _db.MatchmakingTickets
            .FirstOrDefaultAsync(x => x.PlayerId == playerId && x.Status == "QUEUED");

        if (existingTicket != null)
            throw new InvalidOperationException("Player already has an active matchmaking ticket");

        var ticket = new MatchmakingTicket
        {
            Id = Guid.NewGuid(),
            PlayerId = playerId,
            Mode = request.Mode,
            Region = request.Region,
            Mmr = request.Mmr,
            Status = "QUEUED",
            TimeoutAt = DateTimeOffset.UtcNow.AddSeconds(TicketTimeoutSeconds),
            CreatedAt = DateTimeOffset.UtcNow
        };

        _db.MatchmakingTickets.Add(ticket);
        await _db.SaveChangesAsync();

        // Add to Redis sorted set for fast matching
        var dbRedis = _redis.GetDatabase();
        var score = request.Mmr;
        await dbRedis.SortedSetAddAsync($"matchmaking:{request.Mode}:{request.Region}", ticket.Id.ToString(), score);

        return ToResponse(ticket);
    }

    public async Task<MatchmakingTicketResponse?> GetTicketAsync(Guid ticketId)
    {
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

        var dbRedis = _redis.GetDatabase();
        await dbRedis.SortedSetRemoveAsync($"matchmaking:{ticket.Mode}:{ticket.Region}", ticket.Id.ToString());

        return true;
    }

    public async Task ProcessMatchmakingTickAsync()
    {
        var timeoutTickets = await _db.MatchmakingTickets
            .Where(x => x.Status == "QUEUED" && x.TimeoutAt <= DateTimeOffset.UtcNow)
            .ToListAsync();

        foreach (var ticket in timeoutTickets)
        {
            ticket.Status = "TIMEOUT";
            _logger.LogInformation("Matchmaking ticket {TicketId} timed out", ticket.Id);
        }

        if (timeoutTickets.Any())
            await _db.SaveChangesAsync();
    }

    private static MatchmakingTicketResponse ToResponse(MatchmakingTicket t) =>
        new(t.Id, t.PlayerId, t.Mode, t.Region, t.Mmr, t.Status, t.MatchedSessionId, t.CreatedAt, t.TimeoutAt);
}