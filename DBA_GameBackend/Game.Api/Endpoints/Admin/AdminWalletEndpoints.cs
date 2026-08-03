/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：定义 Admin 端点中钱包管理相关接口，包括余额查询、流水查询和余额调整。
- 阅读重点：余额调整会写入 WalletLedger（BizType=ADMIN_ADJUST）并记录审计日志；正数增加，负数扣除。
- 修改提示：调整金额的上下限、币种白名单等应迁移到配置或数据资产驱动。
*/

using Game.Api.Extensions;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.Admin;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.Admin;

public static partial class AdminEndpoints
{
    private static void MapAdminWalletEndpoints(RouteGroupBuilder admin)
    {
        admin.MapGet("/wallet/balances", ListWalletBalances)
            .WithSummary("查询玩家钱包余额列表")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
        admin.MapGet("/wallet/ledgers", ListWalletLedgers)
            .WithSummary("查询钱包流水")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
        admin.MapPost("/wallet/adjust", AdjustWallet)
            .WithSummary("调整玩家钱包余额")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);
    }

    // ==================== 查询玩家钱包余额列表 ====================

    private static async Task<IResult> ListWalletBalances(
        GameDbContext db,
        Guid? playerId = null,
        int page = 1,
        int pageSize = 50)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query = db.WalletBalances.AsNoTracking().AsQueryable();
        if (playerId.HasValue)
        {
            query = query.Where(x => x.PlayerId == playerId.Value);
        }

        var total = await query.CountAsync();
        var items = await query
            .OrderByDescending(x => x.UpdatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new AdminWalletBalanceItem(
                x.Id,
                x.PlayerId,
                x.CurrencyType,
                x.Balance,
                x.UpdatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<AdminWalletBalanceListResponse>.Ok(
            new AdminWalletBalanceListResponse(items, total, page, pageSize)));
    }

    // ==================== 查询钱包流水 ====================

    private static async Task<IResult> ListWalletLedgers(
        GameDbContext db,
        Guid? playerId = null,
        string? bizType = null,
        int page = 1,
        int pageSize = 50)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query = db.WalletLedgers.AsNoTracking().AsQueryable();
        if (playerId.HasValue)
        {
            query = query.Where(x => x.PlayerId == playerId.Value);
        }
        if (!string.IsNullOrWhiteSpace(bizType))
        {
            query = query.Where(x => x.BizType == bizType);
        }

        var total = await query.CountAsync();
        var items = await query
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new AdminWalletLedgerItem(
                x.Id,
                x.PlayerId,
                x.CurrencyType,
                x.Amount,
                x.BalanceBefore,
                x.BalanceAfter,
                x.BizType,
                x.BizId,
                x.IdempotencyKey,
                x.OperatorId,
                x.CreatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<AdminWalletLedgerListResponse>.Ok(
            new AdminWalletLedgerListResponse(items, total, page, pageSize)));
    }

    // ==================== 调整玩家钱包余额 ====================

    private static async Task<IResult> AdjustWallet(
        AdminAdjustWalletRequest request,
        GameDbContext db,
        HttpContext ctx,
        CancellationToken cancellationToken)
    {
        if (request.PlayerId == Guid.Empty)
        {
            return ErrorResponse.BadRequest("playerId 不能为空。").ToProblem();
        }
        if (string.IsNullOrWhiteSpace(request.CurrencyType))
        {
            return ErrorResponse.BadRequest("currencyType 不能为空。").ToProblem();
        }
        if (request.Amount == 0)
        {
            return ErrorResponse.BadRequest("amount 不能为 0。").ToProblem();
        }
        if (string.IsNullOrWhiteSpace(request.Reason))
        {
            return ErrorResponse.BadRequest("调整余额必须填写 reason。").ToProblem();
        }

        var adminId = GetAdminId(ctx);
        var currencyType = request.CurrencyType.Trim();
        var now = DateTimeOffset.UtcNow;

        await using var tx = await db.Database.BeginTransactionAsync(cancellationToken);

        var balance = await db.WalletBalances
            .FirstOrDefaultAsync(x => x.PlayerId == request.PlayerId && x.CurrencyType == currencyType, cancellationToken);

        long balanceBefore = 0;
        if (balance is null)
        {
            if (request.Amount < 0)
            {
                return ErrorResponse.BadRequest("玩家无该币种余额，不能扣除。").ToProblem();
            }
            balance = new WalletBalance
            {
                Id = Guid.NewGuid(),
                PlayerId = request.PlayerId,
                CurrencyType = currencyType,
                Balance = request.Amount,
                UpdatedAt = now
            };
            db.WalletBalances.Add(balance);
        }
        else
        {
            balanceBefore = balance.Balance;
            if (balance.Balance + request.Amount < 0)
            {
                return ErrorResponse.BadRequest($"余额不足，当前余额 {balance.Balance}，尝试扣除 {-request.Amount}。").ToProblem();
            }
            balance.Balance += request.Amount;
            balance.UpdatedAt = now;
        }

        db.WalletLedgers.Add(new WalletLedger
        {
            Id = Guid.NewGuid(),
            PlayerId = request.PlayerId,
            CurrencyType = currencyType,
            Amount = request.Amount,
            BalanceBefore = balanceBefore,
            BalanceAfter = balance.Balance,
            BizType = "ADMIN_ADJUST",
            BizId = adminId?.ToString() ?? string.Empty,
            IdempotencyKey = $"admin-adjust-{Guid.NewGuid():N}",
            OperatorId = adminId,
            CreatedAt = now
        });

        await AddAuditLogAsync(
            db,
            adminId,
            "ADMIN_WALLET_ADJUST",
            "WalletBalance",
            $"{request.PlayerId}:{currencyType}",
            request.Reason.Trim(),
            ctx);

        await db.SaveChangesAsync(cancellationToken);
        await tx.CommitAsync(cancellationToken);

        return Results.Ok(ApiResponse.Ok(new
        {
            playerId = request.PlayerId,
            currencyType,
            amount = request.Amount,
            balanceBefore,
            balanceAfter = balance.Balance
        }, "余额调整成功。"));
    }
}
