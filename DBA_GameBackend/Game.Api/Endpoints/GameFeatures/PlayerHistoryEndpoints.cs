/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：定义 GameFeatures 相关 HTTP 接口路由、鉴权要求、请求解析和统一响应。
- 阅读重点：每个 partial 文件对应一个功能域；总入口只负责聚合注册。
- 修改提示：新增功能时优先放到对应领域文件，避免 GameFeatureEndpoints 再次膨胀。
*/

using Game.Shared.Common;
using Game.Shared.Contracts.GameFeatures;
using Game.Api.Extensions;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;
using System.Security.Claims;
using System.Text.Json;

namespace Game.Api.Endpoints.GameFeatures;
public static partial class GameFeatureEndpoints
{
    private static void MapPlayerHistoryEndpoints(IEndpointRouteBuilder app)
    {
        // 战绩查询
        var matchHistory = app.MapGroup("/api/players/me/matches").WithTags("战绩");
        matchHistory.MapGet("/", GetMatchHistory)
            .WithSummary("获取历史战绩")
            .WithDescription("获取当前玩家的历史比赛记录")
            .RequireAuthorization();

        // 举报系统
        var report = app.MapGroup("/api/reports").WithTags("举报");
        report.MapPost("/", SubmitReport)
            .WithSummary("提交举报")
            .WithDescription("举报违规玩家")
            .RequireAuthorization();

        // 客服工单
        var support = app.MapGroup("/api/support").WithTags("客服");
        support.MapGet("/tickets", GetMyTickets)
            .WithSummary("获取我的工单")
            .WithDescription("获取当前玩家的客服工单")
            .RequireAuthorization();
        support.MapPost("/tickets", CreateTicket)
            .WithSummary("创建工单")
            .WithDescription("创建新的客服工单")
            .RequireAuthorization();
        support.MapGet("/tickets/{ticketId}", GetTicketDetail)
            .WithSummary("获取工单详情")
            .WithDescription("获取客服工单详情及回复")
            .RequireAuthorization();
        support.MapPost("/tickets/{ticketId}/reply", ReplyTicket)
            .WithSummary("回复工单")
            .WithDescription("回复客服工单")
            .RequireAuthorization();
    }

    // ==================== 战绩查询 ====================

    private static async Task<IResult> GetMatchHistory(HttpContext ctx, GameDbContext db, int page = 1, int pageSize = 50)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        (page, pageSize) = NormalizePaging(page, pageSize);

        var totalCount = await db.PlayerMatchHistories.CountAsync(x => x.PlayerId == playerId.Value);

        var histories = await db.PlayerMatchHistories
            .Where(x => x.PlayerId == playerId.Value)
            .OrderByDescending(x => x.PlayedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .ToListAsync();

        var sessionIds = histories.Select(x => x.SessionId).ToList();
        var playerResults = await db.MatchPlayerResults
            .Include(x => x.MatchResult)
            .Where(x => x.PlayerId == playerId.Value &&
                        x.MatchResult != null &&
                        sessionIds.Contains(x.MatchResult.SessionId))
            .ToListAsync();
        var playerResultBySession = playerResults
            .Where(x => x.MatchResult != null)
            .GroupBy(x => x.MatchResult!.SessionId)
            .ToDictionary(
                group => group.Key,
                group => group.OrderByDescending(x => x.CreatedAt).First());

        var matches = histories
            .Select(x =>
            {
                playerResultBySession.TryGetValue(x.SessionId, out var playerResult);
                return new MatchHistoryDto(
                    x.SessionId,
                    x.Mode,
                    x.MapId,
                    x.Team,
                    x.Result,
                    x.Kills,
                    x.Deaths,
                    x.Assists,
                    x.Score,
                    playerResult?.MatchResult?.ResultJson ?? "{}",
                    ExtractWinnerTeam(playerResult?.MatchResult?.ResultJson),
                    x.DurationSeconds,
                    playerResult?.ExpDelta ?? 0,
                    playerResult is null ? new Dictionary<string, object>() : ParseRewardJson(playerResult.RewardJson),
                    x.PlayedAt);
            })
            .ToList();

        return Results.Ok(ApiResponse<MatchHistoryResponse>.Ok(new MatchHistoryResponse(matches, totalCount, page, pageSize)));
    }

    private static string? ExtractWinnerTeam(string? resultJson)
    {
        if (string.IsNullOrWhiteSpace(resultJson))
        {
            return null;
        }

        try
        {
            using var payload = JsonDocument.Parse(resultJson);
            var root = payload.RootElement;
            return TryGetNonEmptyString(root, "winnerTeam")
                ?? TryGetNonEmptyString(root, "winner_team");
        }
        catch (JsonException)
        {
            return null;
        }
    }

    private static string? TryGetNonEmptyString(JsonElement root, string propertyName)
    {
        if (!root.TryGetProperty(propertyName, out var property) ||
            property.ValueKind != JsonValueKind.String)
        {
            return null;
        }

        var value = property.GetString();
        return string.IsNullOrWhiteSpace(value)
            ? null
            : value.Trim();
    }

    private static IReadOnlyDictionary<string, object> ParseRewardJson(string rewardJson)
    {
        if (string.IsNullOrWhiteSpace(rewardJson))
        {
            return new Dictionary<string, object>();
        }

        try
        {
            var rewards = JsonSerializer.Deserialize<Dictionary<string, JsonElement>>(rewardJson);
            if (rewards is null)
            {
                return new Dictionary<string, object>();
            }

            return rewards.ToDictionary(
                pair => pair.Key,
                pair => NormalizeRewardValue(pair.Value));
        }
        catch (JsonException)
        {
            return new Dictionary<string, object>();
        }
    }

    private static object NormalizeRewardValue(JsonElement value)
    {
        return value.ValueKind switch
        {
            JsonValueKind.Number when value.TryGetInt64(out var longValue) => longValue,
            JsonValueKind.Number when value.TryGetDouble(out var doubleValue) => doubleValue,
            JsonValueKind.String => value.GetString() ?? string.Empty,
            JsonValueKind.True => true,
            JsonValueKind.False => false,
            JsonValueKind.Null => string.Empty,
            _ => value.GetRawText()
        };
    }

    // ==================== 举报系统 ====================

    private static async Task<IResult> SubmitReport(SubmitReportRequest request, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var report = new Report
        {
            ReporterId = playerId.Value,
            ReportedPlayerId = request.ReportedPlayerId,
            ReportType = request.ReportType,
            Content = request.Content,
            EvidenceJson = System.Text.Json.JsonSerializer.Serialize(request.EvidenceUrls ?? new List<string>())
        };

        db.Reports.Add(report);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<object>.Ok(new { ReportId = report.Id }));
    }

    // ==================== 客服工单 ====================

    private static async Task<IResult> GetMyTickets(HttpContext ctx, GameDbContext db, int page = 1, int pageSize = 50)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        (page, pageSize) = NormalizePaging(page, pageSize);

        var totalCount = await db.SupportTickets.CountAsync(x => x.PlayerId == playerId.Value);

        var tickets = await db.SupportTickets
            .Where(x => x.PlayerId == playerId.Value)
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new TicketDto(x.Id, x.TicketType, x.Subject, x.Status, x.Priority, x.CreatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<TicketListResponse>.Ok(new TicketListResponse(tickets, totalCount, page, pageSize)));
    }

    private static async Task<IResult> CreateTicket(CreateTicketRequest request, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var ticket = new SupportTicket
        {
            PlayerId = playerId.Value,
            TicketType = request.TicketType,
            Subject = request.Subject,
            Content = request.Content,
            Priority = request.Priority
        };

        db.SupportTickets.Add(ticket);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<object>.Ok(new { TicketId = ticket.Id }));
    }

    private static async Task<IResult> GetTicketDetail(Guid ticketId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var ticket = await db.SupportTickets
            .Include(x => x.Player)
            .FirstOrDefaultAsync(x => x.Id == ticketId && x.PlayerId == playerId.Value);

        if (ticket == null) return ErrorResponse.NotFound("工单不存在").ToProblem();

        var rawReplies = await db.TicketReplies
            .Where(x => x.TicketId == ticketId && !x.IsInternal)
            .OrderBy(x => x.CreatedAt)
            .ToListAsync();

        var playerIds = rawReplies.Where(x => x.PlayerId.HasValue).Select(x => x.PlayerId!.Value).Distinct().ToList();
        var adminIds = rawReplies.Where(x => x.AdminId.HasValue).Select(x => x.AdminId!.Value).Distinct().ToList();
        var playerNames = await db.PlayerProfiles
            .Where(x => playerIds.Contains(x.PlayerId))
            .ToDictionaryAsync(x => x.PlayerId, x => x.Nickname);
        var adminNames = await db.AdminUsers
            .Where(x => adminIds.Contains(x.Id))
            .ToDictionaryAsync(x => x.Id, x => x.Username);

        var replies = rawReplies
            .Select(x => new TicketReplyDto(
                x.Id,
                x.Content,
                x.IsInternal,
                x.PlayerId,
                x.AdminId,
                x.PlayerId.HasValue && playerNames.TryGetValue(x.PlayerId.Value, out var playerName)
                    ? playerName
                    : x.AdminId.HasValue && adminNames.TryGetValue(x.AdminId.Value, out var adminName)
                        ? adminName
                        : null,
                x.CreatedAt))
            .ToList();

        var detail = new TicketDetailDto(
            ticket.Id, ticket.TicketType, ticket.Subject, ticket.Content, ticket.Status,
            ticket.Priority, ticket.CreatedAt, replies);

        return Results.Ok(ApiResponse<TicketDetailDto>.Ok(detail));
    }

    private static async Task<IResult> ReplyTicket(Guid ticketId, ReplyTicketRequest request, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var ticket = await db.SupportTickets.FirstOrDefaultAsync(x => x.Id == ticketId && x.PlayerId == playerId.Value);
        if (ticket == null) return ErrorResponse.NotFound("工单不存在").ToProblem();

        var reply = new TicketReply
        {
            TicketId = ticketId,
            PlayerId = playerId.Value,
            Content = request.Content,
            IsInternal = false
        };

        db.TicketReplies.Add(reply);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse.Ok());
    }
}
