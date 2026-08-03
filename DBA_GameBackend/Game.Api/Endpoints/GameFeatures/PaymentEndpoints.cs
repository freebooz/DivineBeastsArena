/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：定义充值/支付相关 HTTP 接口，包括创建订单、查询订单、第三方支付回调。
- 阅读重点：充值档位为开发阶段常量，后续迁移到数据库或配置；回调接口开发阶段跳过验签。
- 修改提示：接入真实支付后，将验签、订单号生成、第三方下单逻辑下沉到服务层；档位数据迁移到 DataAsset/DataTable。
*/

using System.Text.Json;
using Game.Api.Extensions;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.GameFeatures;
using Game.Shared.Errors;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.GameFeatures;

public static partial class GameFeatureEndpoints
{
    // 充值档位（开发阶段常量，后续迁移到数据库配置）
    // 金额单位：分（CNY），VirtualAmount 单位：虚拟币个数
    private static readonly Dictionary<string, (string Name, long Amount, string Currency, long VirtualAmount, string VirtualCurrency)> RechargeProducts = new()
    {
        ["gem_60"] = ("60 钻石", 600, "CNY", 60, "GEM"),
        ["gem_328"] = ("328 钻石", 3280, "CNY", 328, "GEM"),
        ["gem_648"] = ("648 钻石", 6480, "CNY", 648, "GEM"),
        ["gem_1280"] = ("1280 钻石", 12800, "CNY", 1280, "GEM"),
        ["coin_100"] = ("100 金币", 1000, "CNY", 100, "COIN"),
    };

    // 支持的支付平台白名单
    private static readonly HashSet<string> SupportedPaymentPlatforms = new(StringComparer.OrdinalIgnoreCase)
    {
        "WECHAT", "ALIPAY", "STEAM", "EOS", "MOCK"
    };

    private static void MapPaymentEndpoints(IEndpointRouteBuilder app)
    {
        var payments = app.MapGroup("/api/payments").WithTags("充值/支付");

        payments.MapPost("/create-order", CreatePaymentOrder)
            .WithSummary("创建充值订单")
            .WithDescription("根据充值档位 ID 创建支付订单，返回订单号和支付参数")
            .RequireAuthorization();

        payments.MapGet("/orders/{orderId}", GetPaymentOrder)
            .WithSummary("查询订单状态")
            .WithDescription("根据订单 ID 查询当前玩家的充值订单详情")
            .RequireAuthorization();

        payments.MapGet("/orders", GetMyPaymentOrders)
            .WithSummary("查询我的充值订单列表")
            .WithDescription("分页查询当前玩家的充值订单")
            .RequireAuthorization();

        // 第三方支付回调（无需认证，但需要验签；开发阶段跳过验签）
        payments.MapPost("/callback/{platform}", HandlePaymentCallback)
            .WithSummary("第三方支付回调")
            .WithDescription("接收第三方支付平台回调，更新订单状态并发放虚拟币。开发阶段跳过验签。");
    }

    // ==================== 创建充值订单 ====================

    private static async Task<IResult> CreatePaymentOrder(CreatePaymentOrderRequest request, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        if (string.IsNullOrWhiteSpace(request.ProductId))
        {
            return ErrorResponse.BadRequest("充值档位 ID 不能为空。").ToProblem();
        }

        if (string.IsNullOrWhiteSpace(request.Platform))
        {
            return ErrorResponse.BadRequest("支付平台不能为空。").ToProblem();
        }

        var platform = request.Platform.ToUpperInvariant();
        if (!SupportedPaymentPlatforms.Contains(platform))
        {
            return ErrorResponse.BadRequest($"不支持的支付平台：{platform}。").ToProblem();
        }

        if (!RechargeProducts.TryGetValue(request.ProductId, out var product))
        {
            return ErrorResponse.NotFound($"充值档位不存在：{request.ProductId}").ToProblem();
        }

        var now = DateTimeOffset.UtcNow;
        var order = new PaymentOrder
        {
            Id = Guid.NewGuid(),
            PlayerId = playerId.Value,
            Platform = platform,
            PlatformOrderId = $"{platform.ToLowerInvariant()}-{now:yyyyMMddHHmmss}-{Guid.NewGuid():N}",
            Status = "PENDING",
            Amount = product.Amount,
            Currency = product.Currency,
            ProductId = request.ProductId,
            ProductName = product.Name,
            VirtualAmount = product.VirtualAmount,
            VirtualCurrency = product.VirtualCurrency,
            CreatedAt = now,
            UpdatedAt = now
        };

        db.PaymentOrders.Add(order);
        await db.SaveChangesAsync();

        var productInfo = new PaymentProductInfoDto(
            request.ProductId,
            product.Name,
            product.Amount,
            product.Currency,
            product.VirtualAmount,
            product.VirtualCurrency);

        var response = new PaymentOrderResponse(
            order.Id,
            order.Status,
            order.Amount,
            order.Currency,
            order.Platform,
            order.PlatformOrderId,
            productInfo,
            order.CreatedAt,
            order.PaidAt);

        return Results.Ok(ApiResponse<PaymentOrderResponse>.Ok(response));
    }

    // ==================== 查询订单状态 ====================

    private static async Task<IResult> GetPaymentOrder(Guid orderId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var order = await db.PaymentOrders.FirstOrDefaultAsync(x => x.Id == orderId);
        if (order == null)
        {
            return ErrorResponse.NotFound("订单不存在。").ToProblem();
        }

        if (order.PlayerId != playerId.Value)
        {
            return ErrorResponse.NotFound("订单不存在。").ToProblem();
        }

        var productInfo = new PaymentProductInfoDto(
            order.ProductId,
            order.ProductName,
            order.Amount,
            order.Currency,
            order.VirtualAmount,
            order.VirtualCurrency);

        var response = new PaymentOrderResponse(
            order.Id,
            order.Status,
            order.Amount,
            order.Currency,
            order.Platform,
            order.PlatformOrderId,
            productInfo,
            order.CreatedAt,
            order.PaidAt);

        return Results.Ok(ApiResponse<PaymentOrderResponse>.Ok(response));
    }

    // ==================== 查询我的充值订单列表 ====================

    private static async Task<IResult> GetMyPaymentOrders(HttpContext ctx, GameDbContext db, int page = 1, int pageSize = 50)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        (page, pageSize) = NormalizePaging(page, pageSize);

        var totalCount = await db.PaymentOrders.CountAsync(x => x.PlayerId == playerId.Value);

        var orders = await db.PaymentOrders
            .Where(x => x.PlayerId == playerId.Value)
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new PaymentOrderResponse(
                x.Id,
                x.Status,
                x.Amount,
                x.Currency,
                x.Platform,
                x.PlatformOrderId,
                new PaymentProductInfoDto(
                    x.ProductId,
                    x.ProductName,
                    x.Amount,
                    x.Currency,
                    x.VirtualAmount,
                    x.VirtualCurrency),
                x.CreatedAt,
                x.PaidAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<PaymentOrderListResponse>.Ok(
            new PaymentOrderListResponse(orders, totalCount, page, pageSize)));
    }

    // ==================== 第三方支付回调 ====================

    private static async Task<IResult> HandlePaymentCallback(string platform, HttpContext ctx, GameDbContext db)
    {
        if (string.IsNullOrWhiteSpace(platform))
        {
            return ErrorResponse.BadRequest("平台参数不能为空。").ToProblem();
        }

        var normalizedPlatform = platform.ToUpperInvariant();
        if (!SupportedPaymentPlatforms.Contains(normalizedPlatform))
        {
            return ErrorResponse.BadRequest($"不支持的支付平台：{platform}。").ToProblem();
        }

        // 读取第三方回调原始数据
        string callbackPayload;
        ctx.Request.EnableBuffering();
        ctx.Request.Body.Position = 0;
        using (var reader = new StreamReader(ctx.Request.Body))
        {
            callbackPayload = await reader.ReadToEndAsync();
        }

        if (string.IsNullOrWhiteSpace(callbackPayload))
        {
            return ErrorResponse.BadRequest("回调数据为空。").ToProblem();
        }

        // 开发阶段：从回调数据中解析 orderId 或 platformOrderId
        // 真实环境应根据各平台文档解析签名与字段
        string? orderIdRaw = TryExtractStringField(callbackPayload, "orderId") ?? TryExtractStringField(callbackPayload, "order_id");
        string? platformOrderId = TryExtractStringField(callbackPayload, "platformOrderId")
            ?? TryExtractStringField(callbackPayload, "platform_order_id")
            ?? TryExtractStringField(callbackPayload, "out_trade_no");

        PaymentOrder? order = null;
        if (!string.IsNullOrWhiteSpace(orderIdRaw) && Guid.TryParse(orderIdRaw, out var orderId))
        {
            order = await db.PaymentOrders.FirstOrDefaultAsync(x => x.Id == orderId);
        }

        if (order == null && !string.IsNullOrWhiteSpace(platformOrderId))
        {
            order = await db.PaymentOrders.FirstOrDefaultAsync(x => x.PlatformOrderId == platformOrderId);
        }

        if (order == null)
        {
            return ErrorResponse.NotFound("未匹配到对应的支付订单。").ToProblem();
        }

        // 校验平台一致
        if (!string.Equals(order.Platform, normalizedPlatform, StringComparison.OrdinalIgnoreCase))
        {
            return ErrorResponse.BadRequest("回调平台与订单平台不一致。").ToProblem();
        }

        // 幂等：已 PAID 的订单不重复处理
        if (string.Equals(order.Status, "PAID", StringComparison.OrdinalIgnoreCase))
        {
            return Results.Ok(ApiResponse.Ok("订单已处理，幂等返回。"));
        }

        if (string.Equals(order.Status, "REFUNDED", StringComparison.OrdinalIgnoreCase))
        {
            return ErrorResponse.BadRequest("订单已退款，不能再次回调。").ToProblem();
        }

        var now = DateTimeOffset.UtcNow;

        // 更新订单状态
        var previousStatus = order.Status;
        order.Status = "PAID";
        order.PaidAt = now;
        order.UpdatedAt = now;
        order.CallbackJson = callbackPayload;

        // 充值虚拟币到钱包
        var balance = await db.WalletBalances
            .FirstOrDefaultAsync(x => x.PlayerId == order.PlayerId && x.CurrencyType == order.VirtualCurrency);

        long balanceBefore = 0;
        if (balance == null)
        {
            balance = new WalletBalance
            {
                Id = Guid.NewGuid(),
                PlayerId = order.PlayerId,
                CurrencyType = order.VirtualCurrency,
                Balance = order.VirtualAmount,
                UpdatedAt = now
            };
            db.WalletBalances.Add(balance);
        }
        else
        {
            balanceBefore = balance.Balance;
            balance.Balance += order.VirtualAmount;
            balance.UpdatedAt = now;
        }

        // 记录钱包流水（幂等键保证不重复入账）
        db.WalletLedgers.Add(new WalletLedger
        {
            Id = Guid.NewGuid(),
            PlayerId = order.PlayerId,
            CurrencyType = order.VirtualCurrency,
            Amount = order.VirtualAmount,
            BalanceBefore = balanceBefore,
            BalanceAfter = balance.Balance,
            BizType = "PAYMENT_RECHARGE",
            BizId = order.Id.ToString(),
            IdempotencyKey = $"payment-recharge-{order.Id:N}",
            CreatedAt = now
        });

        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse.Ok(new
        {
            orderId = order.Id,
            status = order.Status,
            previousStatus,
            paidAt = order.PaidAt,
            virtualCurrency = order.VirtualCurrency,
            virtualAmount = order.VirtualAmount,
            balanceAfter = balance.Balance
        }, "回调处理成功。"));
    }

    /// <summary>
    /// 从 JSON 字符串中尝试提取一个字符串字段（容忍普通 JSON 与大小写差异）。
    /// </summary>
    private static string? TryExtractStringField(string payload, string fieldName)
    {
        if (string.IsNullOrWhiteSpace(payload))
        {
            return null;
        }

        try
        {
            using var doc = JsonDocument.Parse(payload);
            if (doc.RootElement.ValueKind != JsonValueKind.Object)
            {
                return null;
            }

            foreach (var prop in doc.RootElement.EnumerateObject())
            {
                if (string.Equals(prop.Name, fieldName, StringComparison.OrdinalIgnoreCase) &&
                    prop.Value.ValueKind == JsonValueKind.String)
                {
                    return prop.Value.GetString();
                }
            }
        }
        catch (JsonException)
        {
            return null;
        }

        return null;
    }
}
