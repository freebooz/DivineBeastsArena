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

namespace Game.Api.Endpoints.GameFeatures;
public static partial class GameFeatureEndpoints
{
    // ==================== 辅助方法 ====================

    private static Guid? GetPlayerId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("player_id") ?? ctx.User.FindFirst(ClaimTypes.NameIdentifier);
        return claim != null && Guid.TryParse(claim.Value, out var id) ? id : null;
    }

    private static Guid? GetAdminId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("admin_id");
        return claim != null && Guid.TryParse(claim.Value, out var id) ? id : null;
    }

    private static (int Page, int PageSize) NormalizePaging(int page, int pageSize, int defaultPageSize = 50, int maxPageSize = 200)
    {
        page = Math.Max(page, 1);
        pageSize = pageSize <= 0 ? defaultPageSize : Math.Clamp(pageSize, 1, maxPageSize);
        return (page, pageSize);
    }

    private static void AddAdminAuditLog(
        GameDbContext db,
        Guid adminId,
        string action,
        string targetType,
        string targetId,
        string reason,
        HttpContext ctx)
    {
        db.AdminAuditLogs.Add(new AdminAuditLog
        {
            AdminUserId = adminId,
            Action = action,
            TargetType = targetType,
            TargetId = targetId,
            Reason = reason.Trim(),
            IpAddress = ctx.Connection.RemoteIpAddress?.ToString(),
            UserAgent = ctx.Request.Headers.UserAgent.ToString()
        });
    }
}
