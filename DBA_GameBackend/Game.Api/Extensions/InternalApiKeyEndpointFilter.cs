/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：为 /internal/* 端点提供统一的内部 API Key 过滤器。
- 阅读重点：所有内部服务调用统一使用 X-Internal-Api-Key 与 InternalApi:Key 配置比较。
- 修改提示：如需调整 header 名或错误语义，请同步所有内部端点测试和生产证据契约。
*/

using Game.Shared.Common;

namespace Game.Api.Extensions;

public static class InternalApiKeyEndpointFilter
{
    public const string HeaderName = "X-Internal-Api-Key";

    public static async ValueTask<object?> RequireInternalApiKey(EndpointFilterInvocationContext context, EndpointFilterDelegate next)
    {
        var unauthorized = Validate(context.HttpContext);
        if (unauthorized is not null)
        {
            return unauthorized;
        }

        return await next(context);
    }

    public static IResult? Validate(HttpContext httpContext)
    {
        var configuration = httpContext.RequestServices.GetRequiredService<IConfiguration>();
        var expected = configuration["InternalApi:Key"];
        var actual = httpContext.Request.Headers[HeaderName].ToString();
        return string.IsNullOrWhiteSpace(expected) || !string.Equals(expected, actual, StringComparison.Ordinal)
            ? ErrorResponse.Unauthorized("Invalid internal api key").ToProblem()
            : null;
    }
}
