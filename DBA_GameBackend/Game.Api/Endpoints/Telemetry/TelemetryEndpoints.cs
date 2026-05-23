/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：提供客户端埋点批量上报 mock 接口，保证本地 UE 客户端启动和验证流程无 404。
- 阅读重点：当前接口只确认接收并返回计数；需要持久化时可引入独立 telemetry 表或消息队列。
- 修改提示：避免在 API 进程中同步处理大批埋点，生产方案应异步入队。
*/

using System.Text.Json;
using Game.Shared.Common;

namespace Game.Api.Endpoints.Telemetry;

public static class TelemetryEndpoints
{
    public static void MapTelemetryEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/telemetry").WithTags("Telemetry");

        group.MapPost("/batch", AcceptTelemetryBatch)
            .WithSummary("Accept a batch of client telemetry events");
    }

    private static IResult AcceptTelemetryBatch(TelemetryBatchRequest request)
    {
        var events = request.Events ?? [];
        return Results.Ok(ApiResponse<TelemetryBatchResponse>.Ok(new TelemetryBatchResponse(
            events.Count,
            DateTimeOffset.UtcNow)));
    }

    private sealed record TelemetryBatchRequest(IReadOnlyList<TelemetryEventRequest>? Events);

    private sealed record TelemetryEventRequest(
        string? EventName,
        string? TimestampUtc,
        Dictionary<string, JsonElement>? Properties);

    private sealed record TelemetryBatchResponse(int Accepted, DateTimeOffset AcceptedAt);
}
