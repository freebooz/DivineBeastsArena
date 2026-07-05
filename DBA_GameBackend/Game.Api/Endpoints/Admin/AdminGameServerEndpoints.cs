using Game.Api.Extensions;
using Game.Infrastructure.Database;
using Game.Shared.Common;
using Game.Shared.Contracts.Admin;
using Game.ServerManagement.DedicatedServers;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.Admin;

public static partial class AdminEndpoints
{
    private static async Task<IResult> ListServers(GameDbContext db, string? status = null, int page = 1, int pageSize = 50)
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
        IDedicatedServerOrchestrator manager,
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
}
