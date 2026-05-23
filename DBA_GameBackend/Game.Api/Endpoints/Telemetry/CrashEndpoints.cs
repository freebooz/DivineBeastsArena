/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义客户端崩溃与日志上报接口，用于本地联调和运营后台问题排查。
- 阅读重点：保持客户端插件的 `/api/crashes/upload` 契约兼容，避免 UE 启动扫描崩溃目录时出现 404。
- 修改提示：生产环境接入对象存储时，可将 Base64 内容落盘或上传后回填 DumpUrl/LogUrl。
*/

using System.Text.Json;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;

namespace Game.Api.Endpoints.Telemetry;

public static class CrashEndpoints
{
    public static void MapCrashEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/crashes").WithTags("Telemetry");

        group.MapPost("/upload", UploadCrash)
            .WithSummary("Upload a client crash or log file");
    }

    private static async Task<IResult> UploadCrash(CrashUploadRequest request, GameDbContext db)
    {
        var fileName = string.IsNullOrWhiteSpace(request.FileName)
            ? "unknown-crash-file"
            : Path.GetFileName(request.FileName.Trim());

        var payloadBytes = EstimateBase64PayloadBytes(request.FileBase64);
        var report = new CrashReport
        {
            ClientVersion = request.ClientVersion,
            Platform = request.Platform ?? "Windows",
            CrashType = request.CrashType ?? InferCrashType(fileName),
            Title = $"Uploaded {fileName}",
            Description = $"Mock crash upload accepted from client. Estimated payload bytes: {payloadBytes}.",
            DumpUrl = $"mock://crashes/{Guid.NewGuid():N}/{fileName}",
            MetadataJson = JsonSerializer.Serialize(new
            {
                fileName,
                request.FilePath,
                payloadBytes,
                acceptedAt = DateTimeOffset.UtcNow
            }),
            CreatedAt = DateTimeOffset.UtcNow
        };

        db.CrashReports.Add(report);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<CrashUploadResponse>.Ok(new CrashUploadResponse(
            report.Id,
            report.DumpUrl!,
            report.CreatedAt)));
    }

    private static string InferCrashType(string fileName)
    {
        var extension = Path.GetExtension(fileName).ToLowerInvariant();
        return extension switch
        {
            ".log" => "CLIENT_LOG",
            ".dmp" => "MINIDUMP",
            ".xml" => "CRASH_CONTEXT",
            _ => "CLIENT_CRASH"
        };
    }

    private static long EstimateBase64PayloadBytes(string? base64)
    {
        if (string.IsNullOrWhiteSpace(base64))
        {
            return 0;
        }

        var padding = base64.EndsWith("==", StringComparison.Ordinal) ? 2 :
            base64.EndsWith("=", StringComparison.Ordinal) ? 1 : 0;
        return Math.Max(0, base64.Length * 3L / 4L - padding);
    }

    private sealed record CrashUploadRequest(
        string? FileName,
        string? FilePath,
        string? FileBase64,
        string? ClientVersion,
        string? Platform,
        string? CrashType);

    private sealed record CrashUploadResponse(Guid CrashReportId, string UploadUrl, DateTimeOffset CreatedAt);
}
