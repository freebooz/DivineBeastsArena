/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Infrastructure.Database;
using Game.Shared.Common;
using Game.Shared.Contracts.Launcher;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.Launcher;

public static class LauncherEndpoints
{
    public static void MapLauncherEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/launcher").WithTags("Launcher");

        group.MapGet("/manifest", GetApiManifest)
            .WithSummary("Get launcher manifest wrapped in ApiResponse");

        group.MapGet("/status", GetStatus)
            .WithSummary("Get launcher service status");

        app.MapGet("/launcher/manifest.json", GetRawManifest)
            .WithTags("Launcher")
            .WithSummary("Get raw launcher manifest for the Tauri launcher");
    }

    private static async Task<IResult> GetApiManifest(string? channel, string? platform, GameDbContext db)
    {
        var manifest = await BuildManifestAsync(channel, platform, db);
        return Results.Ok(ApiResponse<LauncherManifestResponse>.Ok(manifest));
    }

    private static async Task<IResult> GetRawManifest(string? channel, string? platform, GameDbContext db)
    {
        return Results.Ok(await BuildManifestAsync(channel, platform, db));
    }

    private static IResult GetStatus(IConfiguration configuration)
    {
        var apiBaseUrl = configuration["PublicApiBaseUrl"] ?? "http://localhost:8080";
        return Results.Ok(ApiResponse<LauncherStatusResponse>.Ok(new LauncherStatusResponse(
            "ONLINE",
            apiBaseUrl,
            DateTimeOffset.UtcNow,
            new[]
            {
                "当前为开发运营环境。",
                "启动器清单接口已启用。",
                "真实补丁文件列表需要在版本发布流水线中生成。"
            })));
    }

    private static async Task<LauncherManifestResponse> BuildManifestAsync(string? channel, string? platform, GameDbContext db)
    {
        channel = string.IsNullOrWhiteSpace(channel) ? "stable" : channel;
        platform = string.IsNullOrWhiteSpace(platform) ? "Windows" : platform;

        var version = await db.ClientVersions
            .AsNoTracking()
            .Where(x => x.Channel == channel && x.Platform == platform && x.IsActive)
            .OrderByDescending(x => x.CreatedAt)
            .FirstOrDefaultAsync();

        if (version is null)
        {
            return new LauncherManifestResponse(
                "0.1.0",
                channel,
                platform,
                string.Empty,
                false,
                "开发环境默认清单，尚未发布正式客户端包。",
                Array.Empty<LauncherManifestFileDto>());
        }

        return new LauncherManifestResponse(
            version.Version,
            version.Channel,
            version.Platform,
            version.DownloadUrl,
            version.IsMandatory,
            version.ReleaseNotes,
            string.IsNullOrWhiteSpace(version.DownloadUrl)
                ? Array.Empty<LauncherManifestFileDto>()
                : new[]
                {
                    new LauncherManifestFileDto(
                        $"DivineBeastsArena-{version.Platform}-{version.Version}.zip",
                        version.Checksum,
                        version.SizeBytes)
                });
    }
}
