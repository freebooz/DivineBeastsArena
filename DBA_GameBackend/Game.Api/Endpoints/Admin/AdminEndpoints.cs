/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System.IdentityModel.Tokens.Jwt;
using System.Security.Claims;
using System.Text;
using Game.Api.Extensions;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.Admin;
using Game.Shared.Options;
using Game.Worker.ServerManager;
using Microsoft.EntityFrameworkCore;
using Microsoft.IdentityModel.Tokens;

namespace Game.Api.Endpoints.Admin;

public static class AdminEndpoints
{
    public static void MapAdminEndpoints(this IEndpointRouteBuilder app)
    {
        var auth = app.MapGroup("/api/admin/auth").WithTags("Admin Auth");
        auth.MapPost("/login", Login)
            .WithSummary("Admin login")
            .RequireRateLimiting("admin-auth");

        var admin = app.MapGroup("/api/admin")
            .WithTags("Admin")
            .RequireAuthorization()
            .RequireRateLimiting("admin");
        admin.MapGet("/me", GetMe)
            .WithSummary("Get current admin profile");
        admin.MapGet("/players", ListPlayers)
            .WithSummary("List player accounts")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
        admin.MapGet("/players/{playerId:guid}", GetPlayer)
            .WithSummary("Get player detail")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
        admin.MapGet("/audit-logs", ListAuditLogs)
            .WithSummary("List admin audit logs")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);
        admin.MapGet("/feedback", ListFeedback)
            .WithSummary("List player feedback")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
        admin.MapGet("/support/tickets", ListSupportTickets)
            .WithSummary("List support tickets")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
        admin.MapGet("/servers", ListServers)
            .WithSummary("List game servers")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Ops);
        admin.MapPost("/servers/{serverId:guid}/kill", KillServer)
            .WithSummary("Kill game server")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);
        admin.MapGet("/matches", ListMatches)
            .WithSummary("List match results")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
        admin.MapGet("/matches/{matchId:guid}", GetMatch)
            .WithSummary("Get match result detail")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
    }

    private static async Task<IResult> Login(
        AdminLoginRequest request,
        GameDbContext db,
        JwtOptions jwtOptions,
        HttpContext ctx)
    {
        if (string.IsNullOrWhiteSpace(request.Username) || string.IsNullOrWhiteSpace(request.Password))
        {
            return Results.BadRequest(ApiResponse.Fail("Username and password are required."));
        }

        var username = request.Username.Trim();
        var admin = await db.AdminUsers.FirstOrDefaultAsync(x => x.Username == username);
        var now = DateTimeOffset.UtcNow;

        if (admin is null)
        {
            await AddAuditLogAsync(db, null, "ADMIN_LOGIN_FAILED", "AdminUser", username, "Admin user not found", ctx);
            await db.SaveChangesAsync();
            return Results.Unauthorized();
        }

        if (!string.Equals(admin.Status, "ACTIVE", StringComparison.OrdinalIgnoreCase))
        {
            await AddAuditLogAsync(db, admin.Id, "ADMIN_LOGIN_BLOCKED", "AdminUser", admin.Id.ToString(), "Admin disabled", ctx);
            await db.SaveChangesAsync();
            return Results.Unauthorized();
        }

        if (admin.LockedUntil.HasValue && admin.LockedUntil.Value > now)
        {
            await AddAuditLogAsync(db, admin.Id, "ADMIN_LOGIN_BLOCKED", "AdminUser", admin.Id.ToString(), "Admin locked", ctx);
            await db.SaveChangesAsync();
            return Results.Unauthorized();
        }

        var verified = BCrypt.Net.BCrypt.Verify(request.Password, admin.PasswordHash);
        if (!verified)
        {
            admin.FailedLoginCount += 1;
            admin.UpdatedAt = now;
            if (admin.FailedLoginCount >= 5)
            {
                admin.LockedUntil = now.AddMinutes(15);
            }

            await AddAuditLogAsync(db, admin.Id, "ADMIN_LOGIN_FAILED", "AdminUser", admin.Id.ToString(), "Invalid password", ctx);
            await db.SaveChangesAsync();
            return Results.Unauthorized();
        }

        admin.FailedLoginCount = 0;
        admin.LockedUntil = null;
        admin.LastLoginAt = now;
        admin.UpdatedAt = now;

        var token = CreateToken(admin, jwtOptions);
        await AddAuditLogAsync(db, admin.Id, "ADMIN_LOGIN", "AdminUser", admin.Id.ToString(), null, ctx);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<AdminLoginResponse>.Ok(
            new AdminLoginResponse(token, admin.Id, admin.Username, admin.Role)));
    }

    private static async Task<IResult> GetMe(GameDbContext db, HttpContext ctx)
    {
        var adminId = GetAdminId(ctx);
        if (!adminId.HasValue)
        {
            return Results.Unauthorized();
        }

        var admin = await db.AdminUsers
            .Where(x => x.Id == adminId.Value)
            .Select(x => new AdminProfileResponse(x.Id, x.Username, x.Role, x.LastLoginAt))
            .FirstOrDefaultAsync();

        return admin is null
            ? Results.Unauthorized()
            : Results.Ok(ApiResponse<AdminProfileResponse>.Ok(admin));
    }

    private static async Task<IResult> ListPlayers(int page, int pageSize, GameDbContext db)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query = db.PlayerProfiles.AsNoTracking()
            .Include(x => x.PlayerIdentity)
                .ThenInclude(x => x!.Account)
            .Include(x => x.Characters)
            .OrderByDescending(x => x.CreatedAt);

        var total = await query.CountAsync();
        var items = await query
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new AdminPlayerListItem(
                x.PlayerId,
                x.Nickname,
                x.PlayerIdentity == null ? null : x.PlayerIdentity.AccountId,
                x.PlayerIdentity == null || x.PlayerIdentity.Account == null ? "UNKNOWN" : x.PlayerIdentity.Account.AccountType,
                x.PlayerIdentity == null || x.PlayerIdentity.Account == null ? null : x.PlayerIdentity.Account.Email,
                x.PlayerIdentity == null || x.PlayerIdentity.Account == null ? "UNKNOWN" : x.PlayerIdentity.Account.Status,
                x.Level,
                x.Exp,
                x.CreatedAt,
                x.LastLoginAt,
                x.Characters.Count,
                x.Characters
                    .Where(c => c.IsSelected)
                    .Select(c => c.CharacterName)
                    .FirstOrDefault()))
            .ToListAsync();

        return Results.Ok(ApiResponse<AdminPlayerListResponse>.Ok(
            new AdminPlayerListResponse(items, total, page, pageSize)));
    }

    private static async Task<IResult> GetPlayer(Guid playerId, GameDbContext db)
    {
        var player = await db.PlayerProfiles.AsNoTracking()
            .Include(x => x.PlayerIdentity)
                .ThenInclude(x => x!.Account)
            .Include(x => x.Statistics)
            .Include(x => x.InventoryItems)
            .Include(x => x.Unlocks)
            .FirstOrDefaultAsync(x => x.PlayerId == playerId);

        if (player is null)
        {
            return Results.NotFound(ApiResponse.Fail("Player not found."));
        }

        var account = player.PlayerIdentity?.Account;
        var stats = player.Statistics;
        var response = new PlayerDetailResponse(
            player.PlayerId,
            player.Nickname,
            account?.Email,
            account?.AccountType ?? "UNKNOWN",
            account?.Status ?? "UNKNOWN",
            player.Level,
            player.Exp,
            player.LastLoginAt,
            stats is null
                ? new PlayerStatisticsDto(0, 0, 0, 0, 0, 0, 0, 0, 0)
                : new PlayerStatisticsDto(
                    stats.TotalMatches,
                    stats.Wins,
                    stats.Losses,
                    stats.Draws,
                    stats.Kills,
                    stats.Deaths,
                    stats.Assists,
                    stats.Score,
                    stats.PlayTimeSeconds),
            player.InventoryItems
                .OrderBy(x => x.ItemId)
                .Select(x => new InventoryItemDto(x.Id, x.ItemId, x.Quantity, x.ExpiresAt))
                .ToList(),
            player.Unlocks
                .OrderByDescending(x => x.CreatedAt)
                .Select(x => new PlayerUnlockDto(x.UnlockType, x.UnlockId, x.Source, x.CreatedAt))
                .ToList());

        return Results.Ok(ApiResponse<PlayerDetailResponse>.Ok(response));
    }

    private static async Task<IResult> ListAuditLogs(int page, int pageSize, GameDbContext db)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query =
            from log in db.AdminAuditLogs.AsNoTracking()
            join admin in db.AdminUsers.AsNoTracking() on log.AdminUserId equals admin.Id into adminJoin
            from admin in adminJoin.DefaultIfEmpty()
            orderby log.CreatedAt descending
            select new AdminAuditLogItem(
                log.Id,
                log.AdminUserId,
                admin == null ? null : admin.Username,
                log.Action,
                log.TargetType,
                log.TargetId,
                log.Reason,
                log.IpAddress,
                log.CreatedAt);

        var total = await query.CountAsync();
        var items = await query.Skip((page - 1) * pageSize).Take(pageSize).ToListAsync();

        return Results.Ok(ApiResponse<AdminAuditLogListResponse>.Ok(
            new AdminAuditLogListResponse(items, total, page, pageSize)));
    }

    private static async Task<IResult> ListFeedback(int page, int pageSize, string? status, GameDbContext db)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query = db.PlayerFeedbacks.AsNoTracking().AsQueryable();
        if (!string.IsNullOrWhiteSpace(status))
        {
            query = query.Where(x => x.Status == status);
        }

        var total = await query.CountAsync();
        var items = await query
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new AdminFeedbackItem(
                x.Id,
                x.PlayerId,
                x.Nickname,
                x.Email,
                x.FeedbackType,
                x.Title,
                x.Status,
                x.CreatedAt,
                x.UpdatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<AdminFeedbackListResponse>.Ok(
            new AdminFeedbackListResponse(items, total, page, pageSize)));
    }

    private static async Task<IResult> ListSupportTickets(int page, int pageSize, string? status, GameDbContext db)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query = db.SupportTickets.AsNoTracking()
            .Include(x => x.Player)
            .AsQueryable();

        if (!string.IsNullOrWhiteSpace(status))
        {
            query = query.Where(x => x.Status == status);
        }

        var total = await query.CountAsync();
        var items = await query
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new AdminSupportTicketItem(
                x.Id,
                x.PlayerId,
                x.Player == null ? null : x.Player.Nickname,
                x.TicketType,
                x.Subject,
                x.Status,
                x.Priority,
                x.CreatedAt,
                x.UpdatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<AdminSupportTicketListResponse>.Ok(
            new AdminSupportTicketListResponse(items, total, page, pageSize)));
    }

    private static async Task<IResult> ListServers(int page, int pageSize, string? status, GameDbContext db)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query = db.GameServerInstances.AsNoTracking().AsQueryable();
        if (!string.IsNullOrWhiteSpace(status))
        {
            query = query.Where(x => x.Status == status);
        }

        var total = await query.CountAsync();
        var items = await query
            .OrderByDescending(x => x.StartedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new AdminGameServerItem(
                x.Id,
                x.SessionId,
                x.Mode,
                x.MapId,
                x.Region,
                x.BuildVersion,
                x.Ip,
                x.Port,
                x.Status,
                x.StartedAt,
                x.LastHeartbeatAt,
                x.EndedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<AdminGameServerListResponse>.Ok(
            new AdminGameServerListResponse(items, total, page, pageSize)));
    }

    private static async Task<IResult> KillServer(
        Guid serverId,
        KillGameServerRequest request,
        IServerManagerService manager,
        GameDbContext db,
        HttpContext ctx,
        CancellationToken cancellationToken)
    {
        if (string.IsNullOrWhiteSpace(request.Reason))
        {
            return Results.BadRequest(ApiResponse.Fail("高危操作必须填写 reason。"));
        }

        var adminId = GetAdminId(ctx);
        var killed = await manager.KillAsync(serverId, request.Reason.Trim(), cancellationToken);
        if (!killed)
        {
            return Results.NotFound(ApiResponse.Fail("Game server not found."));
        }

        await AddAuditLogAsync(
            db,
            adminId,
            "ADMIN_GAME_SERVER_KILL",
            "GameServerInstance",
            serverId.ToString(),
            request.Reason.Trim(),
            ctx);
        await db.SaveChangesAsync(cancellationToken);

        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> ListMatches(int page, int pageSize, GameDbContext db)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query = db.MatchResults.AsNoTracking()
            .Include(x => x.PlayerResults)
            .OrderByDescending(x => x.CreatedAt);

        var total = await query.CountAsync();
        var items = await query
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new AdminMatchListItem(
                x.Id,
                x.SessionId,
                x.Mode,
                x.MapId,
                x.DurationSeconds,
                x.PlayerResults.Count,
                x.ResultJson,
                x.CreatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<AdminMatchListResponse>.Ok(
            new AdminMatchListResponse(items, total, page, pageSize)));
    }

    private static async Task<IResult> GetMatch(Guid matchId, GameDbContext db)
    {
        var match = await db.MatchResults.AsNoTracking()
            .Include(x => x.PlayerResults)
            .Where(x => x.Id == matchId)
            .Select(x => new AdminMatchDetailResponse(
                x.Id,
                x.SessionId,
                x.Mode,
                x.MapId,
                x.DurationSeconds,
                x.ResultJson,
                x.CreatedAt,
                x.PlayerResults
                    .OrderByDescending(p => p.Score)
                    .Select(p => new AdminMatchPlayerItem(
                        p.PlayerId,
                        p.Team,
                        p.Result,
                        p.Kills,
                        p.Deaths,
                        p.Assists,
                        p.Score,
                        p.ExpDelta))
                    .ToList()))
            .FirstOrDefaultAsync();

        return match is null
            ? Results.NotFound(ApiResponse.Fail("Match result not found."))
            : Results.Ok(ApiResponse<AdminMatchDetailResponse>.Ok(match));
    }

    private static string CreateToken(AdminUser admin, JwtOptions options)
    {
        var claims = new List<Claim>
        {
            new(JwtRegisteredClaimNames.Sub, admin.Id.ToString()),
            new(ClaimTypes.NameIdentifier, admin.Id.ToString()),
            new(ClaimTypes.Name, admin.Username),
            new(ClaimTypes.Role, admin.Role),
            new("admin_id", admin.Id.ToString()),
            new("role", admin.Role)
        };

        var key = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(options.Secret));
        var credentials = new SigningCredentials(key, SecurityAlgorithms.HmacSha256);
        var expires = DateTime.UtcNow.AddMinutes(options.AccessTokenExpiryMinutes);

        var token = new JwtSecurityToken(
            issuer: options.Issuer,
            audience: options.Audience,
            claims: claims,
            expires: expires,
            signingCredentials: credentials);

        return new JwtSecurityTokenHandler().WriteToken(token);
    }

    private static Task AddAuditLogAsync(
        GameDbContext db,
        Guid? adminUserId,
        string action,
        string targetType,
        string? targetId,
        string? reason,
        HttpContext ctx)
    {
        db.AdminAuditLogs.Add(new AdminAuditLog
        {
            AdminUserId = adminUserId,
            Action = action,
            TargetType = targetType,
            TargetId = targetId,
            Reason = reason,
            IpAddress = ctx.Connection.RemoteIpAddress?.ToString(),
            UserAgent = ctx.Request.Headers.UserAgent.ToString()
        });

        return Task.CompletedTask;
    }

    private static Guid? GetAdminId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("admin_id") ?? ctx.User.FindFirst(ClaimTypes.NameIdentifier);
        return claim != null && Guid.TryParse(claim.Value, out var id) ? id : null;
    }

    private static (int Page, int PageSize) NormalizePaging(int page, int pageSize)
    {
        page = Math.Max(page, 1);
        pageSize = pageSize <= 0 ? 50 : Math.Clamp(pageSize, 1, 200);
        return (page, pageSize);
    }
}
