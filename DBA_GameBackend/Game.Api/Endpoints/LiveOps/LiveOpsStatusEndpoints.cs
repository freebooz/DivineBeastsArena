/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Infrastructure.Database;
using Game.Shared.Common;
using Game.Shared.Contracts.LiveOps;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.LiveOps;

public static class LiveOpsStatusEndpoints
{
    private static readonly string[] ActiveServerStatuses = { "STARTING", "READY", "IDLE", "ALLOCATED", "RUNNING" };

    public static void MapLiveOpsStatusEndpoints(this IEndpointRouteBuilder app)
    {
        app.MapGet("/api/live-ops/status", GetStatus)
            .WithTags("LiveOps")
            .WithSummary("Get live operations status summary");

        app.MapGet("/api/operations/status", GetStatus)
            .WithTags("LiveOps")
            .WithSummary("Get live operations status summary");
    }

    private static async Task<IResult> GetStatus(GameDbContext db)
    {
        var now = DateTimeOffset.UtcNow;
        var staleCutoff = now.AddSeconds(-90);

        var totalAccounts = await db.Accounts.CountAsync();
        var totalPlayers = await db.PlayerProfiles.CountAsync();
        var totalCharacters = await db.PlayerCharacters.CountAsync();
        var activeGameServers = await db.GameServerInstances.CountAsync(x => ActiveServerStatuses.Contains(x.Status));
        var staleGameServers = await db.GameServerInstances.CountAsync(x =>
            ActiveServerStatuses.Contains(x.Status) &&
            x.LastHeartbeatAt != null &&
            x.LastHeartbeatAt < staleCutoff);
        var openSupportTickets = await db.SupportTickets.CountAsync(x => x.Status == "OPEN" || x.Status == "IN_PROGRESS");
        var openReports = await db.Reports.CountAsync(x => x.Status == "OPEN" || x.Status == "PROCESSING");
        var activeAnnouncements = await db.Announcements.CountAsync(x =>
            x.IsActive &&
            x.StartAt <= now &&
            (x.EndAt == null || x.EndAt > now));
        var activeEvents = await db.GameEvents.CountAsync(x =>
            x.Status == "ACTIVE" &&
            x.StartAt <= now &&
            (x.EndAt == null || x.EndAt > now));
        var latestVersion = await db.ClientVersions
            .Where(x => x.IsActive)
            .OrderByDescending(x => x.CreatedAt)
            .Select(x => x.Version)
            .FirstOrDefaultAsync() ?? "0.1.0";

        var healthItems = new List<LiveOpsHealthItemDto>
        {
            new("账号系统", totalAccounts > 0 ? "OK" : "WARN", totalAccounts > 0 ? $"已存在 {totalAccounts} 个账号。" : "尚未发现账号数据。"),
            new("角色系统", totalCharacters > 0 ? "OK" : "WARN", totalCharacters > 0 ? $"已存在 {totalCharacters} 个角色。" : "尚未发现角色数据。"),
            new("游戏服务端", staleGameServers == 0 ? "OK" : "WARN", staleGameServers == 0 ? $"活跃服务端 {activeGameServers} 个。" : $"发现 {staleGameServers} 个心跳过期服务端。"),
            new("客服工单", openSupportTickets == 0 ? "OK" : "WARN", openSupportTickets == 0 ? "暂无待处理工单。" : $"待处理工单 {openSupportTickets} 个。"),
            new("举报处理", openReports == 0 ? "OK" : "WARN", openReports == 0 ? "暂无待处理举报。" : $"待处理举报 {openReports} 个。"),
            new("版本发布", latestVersion != "0.1.0" ? "OK" : "WARN", $"当前最新客户端版本：{latestVersion}")
        };

        return Results.Ok(ApiResponse<LiveOpsStatusResponse>.Ok(new LiveOpsStatusResponse(
            now,
            totalAccounts,
            totalPlayers,
            totalCharacters,
            activeGameServers,
            staleGameServers,
            openSupportTickets,
            openReports,
            activeAnnouncements,
            activeEvents,
            latestVersion,
            healthItems)));
    }
}
