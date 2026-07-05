/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Settlement;
using Game.Shared.Common;
using Game.Api.Extensions;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Errors;
using System.Text.Json;

namespace Game.Api.Endpoints.Settlement;

/// <summary>
/// 结算相关接口 / Settlement APIs
/// </summary>
public static class SettlementEndpoints
{
    /// <summary>
    /// 注册结算端点 / Register settlement endpoints
    /// </summary>
    public static void MapSettlementEndpoints(this IEndpointRouteBuilder app)
    {
        var internalGroup = app.MapGroup("/internal/settlement").WithTags("结算(内部)");

        internalGroup.MapPost("/matches/results", SubmitResult)
            .WithSummary("提交比赛结果")
            .WithDescription(@"
游戏服务器在比赛结束后调用此接口提交比赛结果。
系统会自动计算每个玩家的奖励（经验值、道具等）并更新统计数据。

**请求示例：**
```json
{
  ""sessionId"": ""uuid"",
  ""mode"": ""classic"",
  ""mapId"": ""map_001"",
  ""durationSeconds"": 600,
  ""players"": [
    {
      ""playerId"": ""uuid"",
      ""team"": ""blue"",
      ""result"": ""win"",
      ""kills"": 5,
      ""deaths"": 2,
      ""assists"": 3,
      ""score"": 1200
    }
  ]
}
```
");

        internalGroup.MapGet("/matches/results/{matchResultId}", GetResult)
            .WithSummary("获取比赛结果")
            .WithDescription(@"
根据比赛结果ID获取详细的比赛结果信息。

**调用方式：**
GET /internal/settlement/matches/results/{matchResultId}
");

        internalGroup.MapGet("/sessions/{sessionId}/matches/results", GetSessionResults)
            .WithSummary("获取对局结算结果列表")
            .WithDescription(@"
根据对局 Session ID 获取该对局的结算结果列表。结果按创建时间倒序返回，并包含玩家结算明细。

**调用方式：**
GET /internal/settlement/sessions/{sessionId}/matches/results
");
    }

    private static async Task<IResult> SubmitResult(
        SubmitMatchResultRequest request,
        Services.Settlement.ISettlementService svc,
        HttpContext httpContext)
    {
        var unauthorized = InternalApiKeyEndpointFilter.Validate(httpContext);
        if (unauthorized is not null) return unauthorized;

        var result = await svc.SubmitMatchResultAsync(request);
        return result == null
            ? ErrorResponse.BadRequest("Failed to submit match result").ToProblem()
            : Results.Ok(ApiResponse<MatchResultResponse>.Ok(ToResponse(result)));
    }

    private static async Task<IResult> GetResult(
        Guid matchResultId,
        Services.Settlement.ISettlementService svc,
        HttpContext httpContext)
    {
        var unauthorized = InternalApiKeyEndpointFilter.Validate(httpContext);
        if (unauthorized is not null) return unauthorized;

        var result = await svc.GetMatchResultAsync(matchResultId);
        return result == null
            ? ErrorResponse.NotFound(ErrorCodes.MatchResultNotFound).ToProblem()
            : Results.Ok(ApiResponse<MatchResultResponse>.Ok(ToResponse(result)));
    }

    private static async Task<IResult> GetSessionResults(
        Guid sessionId,
        Services.Settlement.ISettlementService svc,
        HttpContext httpContext)
    {
        var unauthorized = InternalApiKeyEndpointFilter.Validate(httpContext);
        if (unauthorized is not null) return unauthorized;

        var results = await svc.GetSessionResultsAsync(sessionId);
        return Results.Ok(ApiResponse<IReadOnlyList<MatchResultResponse>>.Ok(
            results.Select(ToResponse).ToList()));
    }

    private static MatchResultResponse ToResponse(MatchResult result)
    {
        return new MatchResultResponse(
            result.Id,
            result.SessionId,
            result.Mode,
            result.MapId,
            result.DurationSeconds,
            result.ResultJson,
            result.CreatedAt,
            result.PlayerResults.Select(player => new MatchPlayerResultDto(
                player.PlayerId,
                player.Team,
                player.Result,
                player.Kills,
                player.Deaths,
                player.Assists,
                player.Score,
                player.ExpDelta,
                ParseRewardJson(player.RewardJson))).ToList());
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
}
