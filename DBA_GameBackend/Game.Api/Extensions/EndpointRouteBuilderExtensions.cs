/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Common;

namespace Game.Api.Extensions;

public static class EndpointRouteBuilderExtensions
{
    public static RouteHandlerBuilder WithProblemDetails(this RouteHandlerBuilder builder)
    {
        builder.WithTags("Common");
        return builder;
    }

    public static RouteHandlerBuilder WithValidation<T>(this RouteHandlerBuilder builder) where T : class
    {
        return builder.WithTags("Validation");
    }

    public static async Task<IResult> OkOrProblem<T>(this IEndpointRouteBuilder _, Func<Task<T>> action)
    {
        var result = await action();
        return Results.Ok(ApiResponse<T>.Ok(result));
    }

    public static async Task<IResult> OkOrProblem(this IEndpointRouteBuilder _, Func<Task> action)
    {
        await action();
        return Results.Ok(ApiResponse.Ok());
    }
}