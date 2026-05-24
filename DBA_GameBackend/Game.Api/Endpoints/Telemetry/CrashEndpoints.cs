/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义客户端崩溃与日志上报接口，用于本地联调和运营后台问题排查。
- 阅读重点：保持客户端插件的 `/api/crashes/upload` 契约兼容，避免 UE 启动扫描崩溃目录时出现 404。
- 修改提示：默认将 Base64 内容落盘；生产环境接入对象存储时，替换存储后端并回填 DumpUrl/LogUrl。
*/

using System.Text.Json;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;

namespace Game.Api.Endpoints.Telemetry;

public static class CrashEndpoints
{
    private const long DefaultMaxPayloadBytes = 16 * 1024 * 1024;

    public static void MapCrashEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/crashes").WithTags("Telemetry");

        group.MapPost("/upload", UploadCrash)
            .WithSummary("Upload a client crash or log file");
    }

    private static async Task<IResult> UploadCrash(CrashUploadRequest request, GameDbContext db, IConfiguration configuration)
    {
        var fileName = string.IsNullOrWhiteSpace(request.FileName)
            ? "unknown-crash-file"
            : Path.GetFileName(request.FileName.Trim());

        var payloadBytes = EstimateBase64PayloadBytes(request.FileBase64);
        var maxPayloadBytes = configuration.GetValue("CrashUpload:MaxPayloadBytes", DefaultMaxPayloadBytes);
        if (payloadBytes > maxPayloadBytes)
        {
            return Results.BadRequest(ApiResponse.Fail($"Crash payload exceeds max size {maxPayloadBytes} bytes."));
        }

        var createdAt = DateTimeOffset.UtcNow;
        var storedFileName = (string?)null;
        var storagePath = (string?)null;
        var uploadUrl = (string?)null;
        var decodedBytes = Array.Empty<byte>();

        if (!string.IsNullOrWhiteSpace(request.FileBase64))
        {
            try
            {
                decodedBytes = Convert.FromBase64String(request.FileBase64.Trim());
            }
            catch (FormatException)
            {
                return Results.BadRequest(ApiResponse.Fail("FileBase64 is not valid base64."));
            }

            storagePath = ResolveStoragePath(configuration);
            Directory.CreateDirectory(storagePath);
            storedFileName = BuildStoredFileName(fileName, createdAt);
            var storedPath = Path.Combine(storagePath, storedFileName);
            await File.WriteAllBytesAsync(storedPath, decodedBytes);
            uploadUrl = BuildStoredFileUrl(configuration, storedFileName);
        }

        var crashType = request.CrashType ?? InferCrashType(fileName);
        var isLogFile = crashType == "CLIENT_LOG";
        var report = new CrashReport
        {
            ClientVersion = request.ClientVersion,
            Platform = request.Platform ?? "Windows",
            CrashType = crashType,
            Title = $"Uploaded {fileName}",
            Description = payloadBytes > 0
                ? $"Crash upload stored. Payload bytes: {decodedBytes.LongLength}."
                : "Crash metadata accepted without binary payload.",
            DumpUrl = isLogFile ? null : uploadUrl,
            LogUrl = isLogFile ? uploadUrl : null,
            MetadataJson = JsonSerializer.Serialize(new
            {
                fileName,
                storedFileName,
                storagePath,
                request.FilePath,
                payloadBytes,
                acceptedAt = createdAt
            }),
            CreatedAt = createdAt
        };

        db.CrashReports.Add(report);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<CrashUploadResponse>.Ok(new CrashUploadResponse(
            report.Id,
            uploadUrl ?? string.Empty,
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

        var trimmed = base64.Trim();
        var padding = trimmed.EndsWith("==", StringComparison.Ordinal) ? 2 :
            trimmed.EndsWith("=", StringComparison.Ordinal) ? 1 : 0;
        return Math.Max(0, trimmed.Length * 3L / 4L - padding);
    }

    private static string ResolveStoragePath(IConfiguration configuration)
    {
        var configured = configuration["CrashUpload:StoragePath"];
        if (string.IsNullOrWhiteSpace(configured))
            return Path.Combine(AppContext.BaseDirectory, "crash-uploads");

        return Path.IsPathRooted(configured)
            ? configured
            : Path.Combine(AppContext.BaseDirectory, configured);
    }

    private static string BuildStoredFileName(string fileName, DateTimeOffset createdAt)
    {
        var safeName = string.Join("_", fileName.Split(Path.GetInvalidFileNameChars(), StringSplitOptions.RemoveEmptyEntries));
        if (string.IsNullOrWhiteSpace(safeName))
            safeName = "crash-upload.bin";

        return $"{createdAt:yyyyMMddHHmmss}_{Guid.NewGuid():N}_{safeName}";
    }

    private static string BuildStoredFileUrl(IConfiguration configuration, string storedFileName)
    {
        var publicBaseUrl = configuration["CrashUpload:PublicBaseUrl"]?.TrimEnd('/');
        return string.IsNullOrWhiteSpace(publicBaseUrl)
            ? $"local://crash-uploads/{storedFileName}"
            : $"{publicBaseUrl}/{storedFileName}";
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
