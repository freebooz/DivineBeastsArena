/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：提供客户端埋点批量上报接口，并把事件持久化为 JSONL 文件供运营排查和后续日志管道消费。
- 阅读重点：接口保持轻量，只做批量上限校验、事件规范化和追加写入；聚合分析应放到 Worker 或数据平台。
- 修改提示：接入生产日志管道时保持当前 HTTP 契约兼容，替换写入后端即可。
*/

using System.Text.Json;
using System.Text.Json.Serialization;
using Game.Shared.Common;

namespace Game.Api.Endpoints.Telemetry;

public static class TelemetryEndpoints
{
    private const int DefaultMaxEventsPerBatch = 500;
    private static readonly SemaphoreSlim WriteLock = new(1, 1);

    public static void MapTelemetryEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/telemetry").WithTags("Telemetry");

        group.MapPost("/batch", AcceptTelemetryBatch)
            .WithSummary("Accept a batch of client telemetry events");
    }

    private static async Task<IResult> AcceptTelemetryBatch(
        TelemetryBatchRequest request,
        IConfiguration configuration,
        ILoggerFactory loggerFactory)
    {
        var events = request.Events ?? [];
        var maxEvents = configuration.GetValue("TelemetryUpload:MaxEventsPerBatch", DefaultMaxEventsPerBatch);
        if (events.Count > maxEvents)
        {
            return Results.BadRequest(ApiResponse.Fail($"Telemetry batch exceeds max event count {maxEvents}."));
        }

        var acceptedAt = DateTimeOffset.UtcNow;
        if (events.Count > 0)
        {
            var storagePath = ResolveStoragePath(configuration);
            Directory.CreateDirectory(storagePath);

            var records = events.Select(x => new TelemetryPersistedEvent(
                EventName: string.IsNullOrWhiteSpace(x.EventName) ? "unnamed_event" : x.EventName.Trim(),
                TimestampUtc: ParseTimestamp(x.TimestampUtc) ?? acceptedAt,
                PlayerId: request.PlayerId,
                SessionId: request.SessionId,
                ClientVersion: request.ClientVersion,
                Platform: request.Platform,
                Properties: x.Properties ?? [],
                AcceptedAt: acceptedAt));

            var jsonLines = string.Join(Environment.NewLine, records.Select(x => JsonSerializer.Serialize(x))) + Environment.NewLine;
            var filePath = Path.Combine(storagePath, $"{acceptedAt:yyyyMMdd}.jsonl");

            await WriteLock.WaitAsync();
            try
            {
                await File.AppendAllTextAsync(filePath, jsonLines);
            }
            finally
            {
                WriteLock.Release();
            }

            loggerFactory.CreateLogger("TelemetryUpload")
                .LogInformation("Accepted {Count} telemetry events into {FilePath}", events.Count, filePath);
        }

        return Results.Ok(ApiResponse<TelemetryBatchResponse>.Ok(new TelemetryBatchResponse(
            events.Count,
            acceptedAt)));
    }

    private static DateTimeOffset? ParseTimestamp(string? value)
    {
        return DateTimeOffset.TryParse(value, out var timestamp) ? timestamp.ToUniversalTime() : null;
    }

    private static string ResolveStoragePath(IConfiguration configuration)
    {
        var configured = configuration["TelemetryUpload:StoragePath"];
        if (string.IsNullOrWhiteSpace(configured))
            return Path.Combine(AppContext.BaseDirectory, "telemetry-events");

        return Path.IsPathRooted(configured)
            ? configured
            : Path.Combine(AppContext.BaseDirectory, configured);
    }

    private sealed record TelemetryBatchRequest(
        IReadOnlyList<TelemetryEventRequest>? Events,
        Guid? PlayerId,
        Guid? SessionId,
        string? ClientVersion,
        string? Platform);

    private sealed record TelemetryEventRequest(
        string? EventName,
        string? TimestampUtc,
        Dictionary<string, JsonElement>? Properties);

    private sealed record TelemetryBatchResponse(int Accepted, DateTimeOffset AcceptedAt);

    private sealed record TelemetryPersistedEvent(
        [property: JsonPropertyName("eventName")] string EventName,
        [property: JsonPropertyName("timestampUtc")] DateTimeOffset TimestampUtc,
        [property: JsonPropertyName("playerId")] Guid? PlayerId,
        [property: JsonPropertyName("sessionId")] Guid? SessionId,
        [property: JsonPropertyName("clientVersion")] string? ClientVersion,
        [property: JsonPropertyName("platform")] string? Platform,
        [property: JsonPropertyName("properties")] Dictionary<string, JsonElement> Properties,
        [property: JsonPropertyName("acceptedAt")] DateTimeOffset AcceptedAt);
}
