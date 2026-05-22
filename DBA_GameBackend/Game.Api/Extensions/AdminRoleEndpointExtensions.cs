/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：为 Minimal API 端点提供 GM 后台角色校验过滤器，统一 SUPER_ADMIN / OPS / SUPPORT / VIEWER 的权限判断。
- 阅读重点：SUPER_ADMIN 和旧版 Admin 角色被视为最高权限；其它角色必须命中端点声明的允许列表。
- 修改提示：新增后台角色或调整权限矩阵时，优先修改本文件常量与端点上的 RequireAdminRoles 调用。
*/

using System.Security.Claims;
using Game.Shared.Common;

namespace Game.Api.Extensions;

public static class AdminRoleEndpointExtensions
{
    public const string SuperAdmin = "SUPER_ADMIN";
    public const string Ops = "OPS";
    public const string Support = "SUPPORT";
    public const string Viewer = "VIEWER";

    public static RouteHandlerBuilder RequireAdminRoles(this RouteHandlerBuilder builder, params string[] allowedRoles)
    {
        return builder.AddEndpointFilter(async (context, next) =>
        {
            if (IsAllowed(context.HttpContext.User, allowedRoles))
            {
                return await next(context);
            }

            return ErrorResponse.Forbidden("当前管理员角色无权执行该操作。").ToProblem();
        });
    }

    private static bool IsAllowed(ClaimsPrincipal user, IReadOnlyCollection<string> allowedRoles)
    {
        if (allowedRoles.Count == 0)
        {
            return true;
        }

        var roles = user.FindAll(ClaimTypes.Role)
            .Concat(user.FindAll("role"))
            .Select(x => x.Value)
            .Where(x => !string.IsNullOrWhiteSpace(x))
            .Select(x => x.Trim())
            .ToHashSet(StringComparer.OrdinalIgnoreCase);

        if (roles.Contains(SuperAdmin) || roles.Contains("Admin"))
        {
            return true;
        }

        return roles.Overlaps(allowedRoles);
    }
}
