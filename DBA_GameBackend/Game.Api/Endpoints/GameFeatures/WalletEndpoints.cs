/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：定义钱包相关 HTTP 接口，包括查询余额、查询流水。
- 阅读重点：余额和流水数据来自 WalletBalance / WalletLedger 表；只读接口，无写入操作。
- 修改提示：如需新增 GM 调整余额接口，应使用幂等键并补充管理员审计日志。
*/

using Game.Api.Extensions;
using Game.Infrastructure.Database;
using Game.Shared.Common;
using Game.Shared.Contracts.GameFeatures;
using Game.Shared.Errors;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.GameFeatures;

public static partial class GameFeatureEndpoints
{
    private static void MapWalletEndpoints(IEndpointRouteBuilder app)
    {
        var wallet = app.MapGroup("/api/wallet").WithTags("钱包");

        wallet.MapGet("/balance", GetMyWalletBalance)
            .WithSummary("查询我的钱包余额")
            .WithDescription("查询当前玩家所有币种的钱包余额")
            .RequireAuthorization();

        wallet.MapGet("/ledger", GetMyWalletLedger)
            .WithSummary("查询我的钱包流水")
            .WithDescription("分页查询当前玩家的钱包流水记录")
            .RequireAuthorization();
    }

    // ==================== 查询我的钱包余额 ====================

    private static async Task<IResult> GetMyWalletBalance(HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var balances = await db.WalletBalances
            .Where(x => x.PlayerId == playerId.Value)
            .OrderBy(x => x.CurrencyType)
            .Select(x => new WalletBalanceDto(x.CurrencyType, x.Balance, x.UpdatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<WalletBalanceResponse>.Ok(
            new WalletBalanceResponse(playerId.Value, balances)));
    }

    // ==================== 查询我的钱包流水 ====================

    private static async Task<IResult> GetMyWalletLedger(HttpContext ctx, GameDbContext db, int page = 1, int pageSize = 50)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        (page, pageSize) = NormalizePaging(page, pageSize);

        var totalCount = await db.WalletLedgers.CountAsync(x => x.PlayerId == playerId.Value);

        var ledgers = await db.WalletLedgers
            .Where(x => x.PlayerId == playerId.Value)
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new WalletLedgerDto(
                x.Id,
                x.CurrencyType,
                x.Amount,
                x.BalanceBefore,
                x.BalanceAfter,
                x.BizType,
                x.BizId,
                x.CreatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<WalletLedgerResponse>.Ok(
            new WalletLedgerResponse(ledgers, totalCount, page, pageSize)));
    }
}
