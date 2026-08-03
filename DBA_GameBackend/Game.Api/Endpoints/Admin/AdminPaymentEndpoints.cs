/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：定义 Admin 端点中支付订单管理相关接口，包括订单列表、订单详情和退款。
- 阅读重点：退款操作会扣除已充值的虚拟币并记录钱包流水与审计日志；已退款订单幂等返回。
- 修改提示：接入真实支付平台后，退款逻辑应下沉到服务层并补充第三方退款调用。
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
    private static void MapAdminPaymentEndpoints(RouteGroupBuilder admin)
    {
        admin.MapGet("/payments/orders", ListPaymentOrders)
            .WithSummary("查询支付订单列表")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
        admin.MapGet("/payments/orders/{orderId:guid}", GetPaymentOrderDetail)
            .WithSummary("查询支付订单详情")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
        admin.MapPost("/payments/orders/{orderId:guid}/refund", RefundPaymentOrder)
            .WithSummary("退款支付订单")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);
    }

    // ==================== 查询支付订单列表 ====================

    private static async Task<IResult> ListPaymentOrders(
        GameDbContext db,
        string? status = null,
        Guid? playerId = null,
        int page = 1,
        int pageSize = 50)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query = db.PaymentOrders.AsNoTracking().AsQueryable();
        if (!string.IsNullOrWhiteSpace(status))
        {
            query = query.Where(x => x.Status == status);
        }
        if (playerId.HasValue)
        {
            query = query.Where(x => x.PlayerId == playerId.Value);
        }

        var total = await query.CountAsync();
        var items = await query
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new AdminPaymentOrderListItem(
                x.Id,
                x.PlayerId,
                x.Platform,
                x.PlatformOrderId,
                x.Status,
                x.Amount,
                x.Currency,
                x.ProductId,
                x.ProductName,
                x.VirtualAmount,
                x.VirtualCurrency,
                x.CreatedAt,
                x.PaidAt,
                x.UpdatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<AdminPaymentOrderListResponse>.Ok(
            new AdminPaymentOrderListResponse(items, total, page, pageSize)));
    }

    // ==================== 查询支付订单详情 ====================

    private static async Task<IResult> GetPaymentOrderDetail(Guid orderId, GameDbContext db)
    {
        var order = await db.PaymentOrders.AsNoTracking()
            .Where(x => x.Id == orderId)
            .Select(x => new AdminPaymentOrderDetailResponse(
                x.Id,
                x.PlayerId,
                x.Platform,
                x.PlatformOrderId,
                x.Status,
                x.Amount,
                x.Currency,
                x.ProductId,
                x.ProductName,
                x.VirtualAmount,
                x.VirtualCurrency,
                x.CallbackJson,
                x.CreatedAt,
                x.PaidAt,
                x.UpdatedAt))
            .FirstOrDefaultAsync();

        return order is null
            ? ErrorResponse.NotFound("支付订单不存在。").ToProblem()
            : Results.Ok(ApiResponse<AdminPaymentOrderDetailResponse>.Ok(order));
    }

    // ==================== 退款支付订单 ====================

    private static async Task<IResult> RefundPaymentOrder(
        Guid orderId,
        AdminRefundPaymentRequest request,
        GameDbContext db,
        HttpContext ctx,
        CancellationToken cancellationToken)
    {
        if (string.IsNullOrWhiteSpace(request.Reason))
        {
            return ErrorResponse.BadRequest("退款必须填写 reason。").ToProblem();
        }

        var adminId = GetAdminId(ctx);
        var order = await db.PaymentOrders.FirstOrDefaultAsync(x => x.Id == orderId, cancellationToken);
        if (order is null)
        {
            return ErrorResponse.NotFound("支付订单不存在。").ToProblem();
        }

        // 幂等处理：已退款的订单不重复处理
        if (string.Equals(order.Status, "REFUNDED", StringComparison.OrdinalIgnoreCase))
        {
            return Results.Ok(ApiResponse.Ok("订单已退款，幂等返回。"));
        }

        // 仅已支付订单可退款
        if (!string.Equals(order.Status, "PAID", StringComparison.OrdinalIgnoreCase))
        {
            return ErrorResponse.BadRequest($"订单当前状态为 {order.Status}，仅 PAID 状态可退款。").ToProblem();
        }

        await using var tx = await db.Database.BeginTransactionAsync(cancellationToken);
        var now = DateTimeOffset.UtcNow;

        order.Status = "REFUNDED";
        order.UpdatedAt = now;

        // 扣除已充值的虚拟币
        var balance = await db.WalletBalances
            .FirstOrDefaultAsync(x => x.PlayerId == order.PlayerId && x.CurrencyType == order.VirtualCurrency, cancellationToken);

        long balanceBefore = 0;
        if (balance is null)
        {
            // 余额记录不存在时视为 0；退款扣除后为负数视为坏账，但仍记录流水以便对账
            balance = new WalletBalance
            {
                Id = Guid.NewGuid(),
                PlayerId = order.PlayerId,
                CurrencyType = order.VirtualCurrency,
                Balance = -order.VirtualAmount,
                UpdatedAt = now
            };
            db.WalletBalances.Add(balance);
        }
        else
        {
            balanceBefore = balance.Balance;
            balance.Balance -= order.VirtualAmount;
            balance.UpdatedAt = now;
        }

        db.WalletLedgers.Add(new WalletLedger
        {
            Id = Guid.NewGuid(),
            PlayerId = order.PlayerId,
            CurrencyType = order.VirtualCurrency,
            Amount = -order.VirtualAmount,
            BalanceBefore = balanceBefore,
            BalanceAfter = balance.Balance,
            BizType = "PAYMENT_REFUND",
            BizId = order.Id.ToString(),
            IdempotencyKey = $"payment-refund-{order.Id:N}",
            OperatorId = adminId,
            CreatedAt = now
        });

        await AddAuditLogAsync(
            db,
            adminId,
            "ADMIN_PAYMENT_REFUND",
            "PaymentOrder",
            order.Id.ToString(),
            request.Reason.Trim(),
            ctx);

        await db.SaveChangesAsync(cancellationToken);
        await tx.CommitAsync(cancellationToken);

        return Results.Ok(ApiResponse.Ok(new
        {
            orderId = order.Id,
            status = order.Status,
            refundedVirtualAmount = order.VirtualAmount,
            refundedVirtualCurrency = order.VirtualCurrency,
            balanceAfter = balance.Balance
        }, "退款处理成功。"));
    }
}
