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
/// <summary>
/// 游戏功能聚合接口 / Game Feature APIs
/// </summary>
public static partial class GameFeatureEndpoints
{
    /// <summary>
    /// 注册游戏功能端点 / Register game feature endpoints
    /// </summary>
    public static void MapGameFeatureEndpoints(this IEndpointRouteBuilder app)
    {
        MapInventoryEndpoints(app);
        MapRankingEndpoints(app);
        MapSocialEndpoints(app);
        MapCommerceEndpoints(app);
        MapLiveContentEndpoints(app);
        MapPlayerHistoryEndpoints(app);
        MapVersionAnalyticsEndpoints(app);
        MapSessionReconnectEndpoints(app);
        MapPaymentEndpoints(app);
        MapWalletEndpoints(app);
        MapQuestEndpoints(app);
    }
}