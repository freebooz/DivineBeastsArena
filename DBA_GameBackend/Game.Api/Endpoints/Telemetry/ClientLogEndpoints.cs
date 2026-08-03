/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：提供客户端日志上传接口，将日志事件持久化为 CrashReport 记录（CrashType = "CLIENT_LOG"），便于运营排查客户端问题。
- 阅读重点：保持客户端调用契约 /api/client-logs/upload 稳定，仅做基本校验和持久化，复杂聚合交给后台数据管道。
- 修改提示：接入生产日志管道时保持 HTTP 契约兼容，替换写入后端即可。
*/

using System.Text.Json;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;

namespace Game.Api.Endpoints.Telemetry;

/// <summary>
/// 客户端日志上传接口 / Client log upload endpoints
/// </summary>
public static class ClientLogEndpoints
{
    public static void MapClientLogEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/client-logs").WithTags("客户端日志");

        group.MapPost("/upload", UploadClientLogs)
            .WithSummary("上传客户端日志");
    }

    private static async Task<IResult> UploadClientLogs(
        ClientLogUploadRequest request,
        GameDbContext db,
        ILoggerFactory loggerFactory)
    {
        var playerId = GetPlayerId(request.PlayerId);
        var logs = request.Logs ?? [];
        var acceptedAt = DateTimeOffset.UtcNow;

        // 将日志事件序列化为 JSON 存入 CrashReport.MetadataJson
        // CrashType 标记为 CLIENT_LOG，便于与崩溃报告区分
        var metadata = new
        {
            logCount = logs.Count,
            clientVersion = request.ClientVersion,
            platform = request.Platform,
            sessionId = request.SessionId,
            playerId = playerId,
            acceptedAt = acceptedAt,
            logs = logs.Select(x => new
            {
                timestamp = x.Timestamp,
                level = string.IsNullOrWhiteSpace(x.Level) ? "INFO" : x.Level!.Trim().ToUpperInvariant(),
                message = x.Message,
                properties = x.Properties
            })
        };

        var report = new CrashReport
        {
            PlayerId = playerId,
            ClientVersion = request.ClientVersion,
            Platform = request.Platform ?? "Windows",
            CrashType = "CLIENT_LOG",
            Title = $"Client log upload ({logs.Count} entries)",
            Description = $"客户端日志上报，共 {logs.Count} 条记录。",
            DumpUrl = null,
            LogUrl = null,
            MetadataJson = JsonSerializer.Serialize(metadata),
            CreatedAt = acceptedAt
        };

        db.CrashReports.Add(report);
        await db.SaveChangesAsync();

        loggerFactory.CreateLogger("ClientLogUpload")
            .LogInformation("已接收客户端日志上报：玩家={PlayerId}, 条数={Count}, 报告Id={ReportId}",
                playerId, logs.Count, report.Id);

        return Results.Ok(ApiResponse<ClientLogUploadResponse>.Ok(new ClientLogUploadResponse(
            report.Id,
            logs.Count,
            acceptedAt)));
    }

    private static Guid? GetPlayerId(Guid? playerId)
    {
        return playerId.HasValue && playerId.Value != Guid.Empty ? playerId : null;
    }

    private sealed record ClientLogUploadRequest(
        IReadOnlyList<ClientLogEntry>? Logs,
        Guid? PlayerId,
        Guid? SessionId,
        string? ClientVersion,
        string? Platform);

    private sealed record ClientLogEntry(
        string? Timestamp,
        string? Level,
        string? Message,
        Dictionary<string, JsonElement>? Properties);

    private sealed record ClientLogUploadResponse(
        Guid ReportId,
        int Accepted,
        DateTimeOffset AcceptedAt);
}
