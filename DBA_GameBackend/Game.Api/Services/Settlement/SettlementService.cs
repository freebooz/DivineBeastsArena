/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Settlement;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Api.Services.Inventory;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Services.Settlement;

public interface ISettlementService
{
    Task<MatchResult?> SubmitMatchResultAsync(SubmitMatchResultRequest request);
    Task<MatchResult?> GetMatchResultAsync(Guid matchResultId);
    Task<IReadOnlyList<MatchResult>> GetSessionResultsAsync(Guid sessionId);
}

public sealed class SettlementService : ISettlementService
{
    private readonly GameDbContext _db;
    private readonly IInventoryService _inventory;
    private readonly ILogger<SettlementService> _logger;

    public SettlementService(GameDbContext db, IInventoryService inventory, ILogger<SettlementService> logger)
    {
        _db = db;
        _inventory = inventory;
        _logger = logger;
    }

    public async Task<MatchResult?> SubmitMatchResultAsync(SubmitMatchResultRequest request)
    {
        await using var tx = await _db.Database.BeginTransactionAsync();

        var existing = await _db.MatchResults
            .FirstOrDefaultAsync(x => x.SessionId == request.SessionId && x.IdempotencyKey == request.IdempotencyKey);
        if (existing != null) return existing;

        var session = await _db.GameSessions
            .Include(x => x.PlayerSessions)
            .FirstOrDefaultAsync(x => x.Id == request.SessionId);
        if (session == null) return null;
        if (session.Status is not ("IN_PROGRESS" or "SETTLING")) return null;

        var server = await _db.GameServerInstances
            .FirstOrDefaultAsync(x => x.SessionId == request.SessionId && x.Id == session.ServerId);
        if (server == null) return null;

        var sessionPlayers = session.PlayerSessions.Select(x => x.PlayerId).ToHashSet();
        if (request.Players.Any(x => !sessionPlayers.Contains(x.PlayerId))) return null;

        var duration = (int)(DateTimeOffset.UtcNow - (session.StartedAt ?? session.CreatedAt)).TotalSeconds;

        var result = new MatchResult
        {
            Id = Guid.NewGuid(),
            SessionId = request.SessionId,
            ServerId = server.Id,
            Mode = session.Mode,
            MapId = session.MapId,
            DurationSeconds = duration,
            ResultJson = request.ResultJson,
            IdempotencyKey = request.IdempotencyKey,
            CreatedAt = DateTimeOffset.UtcNow
        };

        _db.MatchResults.Add(result);

        foreach (var player in request.Players)
        {
            var playerResult = new MatchPlayerResult
            {
                Id = Guid.NewGuid(),
                MatchResultId = result.Id,
                PlayerId = player.PlayerId,
                Team = player.Team,
                Result = player.Result,
                Kills = player.Kills,
                Deaths = player.Deaths,
                Assists = player.Assists,
                Score = player.Score,
                ExpDelta = player.ExpDelta,
                RewardJson = System.Text.Json.JsonSerializer.Serialize(player.Rewards),
                CreatedAt = DateTimeOffset.UtcNow
            };
            _db.MatchPlayerResults.Add(playerResult);

            var profile = await _db.PlayerProfiles.FirstOrDefaultAsync(x => x.PlayerId == player.PlayerId);
            if (profile != null)
            {
                profile.Exp += player.ExpDelta;
                profile.Level = Math.Max(profile.Level, 1 + (int)(profile.Exp / 1000));
            }

            var stats = await _db.PlayerStatistics.FirstOrDefaultAsync(x => x.PlayerId == player.PlayerId);
            if (stats != null)
            {
                stats.TotalMatches += 1;
                stats.Wins += player.Result.Equals("win", StringComparison.OrdinalIgnoreCase) ? 1 : 0;
                stats.Losses += player.Result.Equals("loss", StringComparison.OrdinalIgnoreCase) ? 1 : 0;
                stats.Draws += player.Result.Equals("draw", StringComparison.OrdinalIgnoreCase) ? 1 : 0;
                stats.Kills += player.Kills;
                stats.Deaths += player.Deaths;
                stats.Assists += player.Assists;
                stats.Score += player.Score;
                stats.PlayTimeSeconds += duration;
                stats.UpdatedAt = DateTimeOffset.UtcNow;
            }

            await _inventory.GrantRewardsAsync(player.PlayerId, player.Rewards, "MATCH_REWARD", result.Id.ToString());
        }

        session.Status = "COMPLETED";
        session.EndedAt = DateTimeOffset.UtcNow;
        session.UpdatedAt = DateTimeOffset.UtcNow;
        server.Status = "ENDING";
        server.UpdatedAt = DateTimeOffset.UtcNow;

        _db.SessionEvents.Add(new SessionEvent
        {
            Id = Guid.NewGuid(),
            GameSessionId = session.Id,
            EventType = "SETTLEMENT_COMPLETED",
            PayloadJson = $"{{\"matchResultId\":\"{result.Id}\"}}",
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        await tx.CommitAsync();

        _logger.LogInformation("Match result submitted for session {SessionId}", request.SessionId);
        return result;
    }

    public async Task<MatchResult?> GetMatchResultAsync(Guid matchResultId)
    {
        return await _db.MatchResults.FindAsync(matchResultId);
    }

    public async Task<IReadOnlyList<MatchResult>> GetSessionResultsAsync(Guid sessionId)
    {
        return await _db.MatchResults
            .Where(x => x.SessionId == sessionId)
            .Include(x => x.PlayerResults)
            .ToListAsync();
    }
}
