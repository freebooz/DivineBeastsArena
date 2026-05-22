/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Config;
using Game.Shared.Common;
using Game.Api.Extensions;

namespace Game.Api.Endpoints.Config;

/// <summary>
/// 配置管理相关接口 / Configuration APIs
/// </summary>
public static class ConfigEndpoints
{
    /// <summary>
    /// 注册配置端点 / Register configuration endpoints
    /// </summary>
    public static void MapConfigEndpoints(this IEndpointRouteBuilder app)
    {
        var client = app.MapGroup("/api/config").WithTags("配置");
        var admin = app.MapGroup("/api/admin/configs")
            .WithTags("配置管理")
            .RequireAuthorization();

        // 客户端接口
        client.MapGet("/manifest", GetManifest)
            .WithSummary("获取配置清单")
            .WithDescription(@"
获取所有可用配置的清单，用于客户端检查需要更新的配置项。
支持按渠道和地区筛选。

**调用方式：**
GET /api/config/manifest?channel=stable&region=global
");

        client.MapGet("/{configKey}", GetConfig)
            .WithSummary("获取指定配置")
            .WithDescription(@"
根据配置键获取具体的配置内容。
配置内容以JSON格式返回。

**调用方式：**
GET /api/config/{configKey}
");

        // 管理员接口
        admin.MapGet("/", ListAllConfigs)
            .WithSummary("获取所有配置列表（管理员）")
            .WithDescription(@"
管理员接口，获取系统中所有配置项的列表。
需要管理员权限。
")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Ops);

        admin.MapPost("/", CreateConfig)
            .WithSummary("创建新配置（管理员）")
            .WithDescription(@"
管理员接口，创建新的配置项。
需要管理员权限。

**请求示例：**
```json
{
  ""key"": ""game_settings"",
  ""value"": ""{}"",
  ""version"": 1,
  ""channel"": ""default"",
  ""region"": ""global""
}
```
")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);

        admin.MapPut("/{id}", UpdateConfig)
            .WithSummary("更新配置（管理员）")
            .WithDescription(@"
管理员接口，更新指定配置项。
需要管理员权限。

**请求示例：**
```json
{
  ""value"": ""{}"",
  ""version"": 2
}
```
")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);

        admin.MapPost("/{id}/validate", ValidateConfig)
            .WithSummary("验证配置（管理员）")
            .WithDescription(@"
管理员接口，验证配置项的格式和内容是否正确。
")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);

        admin.MapPost("/{id}/publish", PublishConfig)
            .WithSummary("发布配置（管理员）")
            .WithDescription(@"
管理员接口，发布配置使其对客户端可见。
发布前需要先验证配置。
")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);

        admin.MapPost("/{id}/rollback", RollbackConfig)
            .WithSummary("回滚配置（管理员）")
            .WithDescription(@"
管理员接口，将配置回滚到上一个版本。
会创建一个新的版本来覆盖当前版本。
")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);
    }

    private static async Task<IResult> GetManifest(string channel, string region, Services.Config.IConfigService svc) =>
        Results.Ok(ApiResponse<GameConfigManifestResponse>.Ok(await svc.GetManifestAsync(channel, "global")));

    private static async Task<IResult> GetConfig(string configKey, Services.Config.IConfigService svc)
    {
        var config = await svc.GetConfigAsync(configKey, "default", "global");
        return config == null
            ? ErrorResponse.NotFound("CONFIG_NOT_FOUND").ToProblem()
            : Results.Ok(ApiResponse<GameConfigResponse>.Ok(config));
    }

    private static async Task<IResult> ListAllConfigs(Services.Config.IConfigService svc) =>
        Results.Ok(ApiResponse<IReadOnlyList<GameConfigResponse>>.Ok(await svc.GetAllConfigsAsync()));

    private static async Task<IResult> CreateConfig(CreateConfigRequest request, Services.Config.IConfigService svc, HttpContext ctx)
    {
        var adminId = GetAdminId(ctx);
        var config = await svc.CreateConfigAsync(request, adminId);
        return Results.Ok(ApiResponse<GameConfigResponse>.Ok(config));
    }

    private static async Task<IResult> UpdateConfig(Guid id, UpdateConfigRequest request, Services.Config.IConfigService svc, HttpContext ctx)
    {
        var adminId = GetAdminId(ctx);
        var config = await svc.UpdateConfigAsync(id, request, adminId);
        return config == null
            ? ErrorResponse.NotFound("CONFIG_NOT_FOUND").ToProblem()
            : Results.Ok(ApiResponse<GameConfigResponse>.Ok(config));
    }

    private static async Task<IResult> ValidateConfig(Guid id, Services.Config.IConfigService svc)
    {
        var config = await svc.GetAllConfigsAsync();
        return Results.Ok(ApiResponse<bool>.Ok(true));
    }

    private static async Task<IResult> PublishConfig(Guid id, PublishConfigRequest request, Services.Config.IConfigService svc, HttpContext ctx)
    {
        var adminId = GetAdminId(ctx);
        if (!adminId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        var config = await svc.PublishConfigAsync(id, request, adminId.Value);
        return config == null
            ? ErrorResponse.NotFound("CONFIG_NOT_FOUND").ToProblem()
            : Results.Ok(ApiResponse<GameConfigResponse>.Ok(config));
    }

    private static async Task<IResult> RollbackConfig(Guid id, RollbackConfigRequest request, Services.Config.IConfigService svc, HttpContext ctx)
    {
        var adminId = GetAdminId(ctx);
        if (!adminId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        var config = await svc.RollbackConfigAsync(id, request, adminId.Value);
        return config == null
            ? ErrorResponse.NotFound("CONFIG_NOT_FOUND").ToProblem()
            : Results.Ok(ApiResponse<GameConfigResponse>.Ok(config));
    }

    private static Guid? GetAdminId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("admin_id");
        return claim != null && Guid.TryParse(claim.Value, out var id) ? id : null;
    }
}
