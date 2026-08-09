/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：实现 ASP.NET Core 请求管线中的横切逻辑，例如异常处理、TraceId 和日志上下文。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System.Net;
using Game.Shared.Common;

namespace Game.Api.Middleware;

public sealed class ExceptionHandlingMiddleware
{
    private readonly RequestDelegate _next;
    private readonly ILogger<ExceptionHandlingMiddleware> _logger;

    public ExceptionHandlingMiddleware(RequestDelegate next, ILogger<ExceptionHandlingMiddleware> logger)
    {
        _next = next;
        _logger = logger;
    }

    public async Task InvokeAsync(HttpContext context)
    {
        try
        {
            await _next(context);
        }
        catch (Exception ex)
        {
            await HandleExceptionAsync(context, ex);
        }
    }

    private async Task HandleExceptionAsync(HttpContext context, Exception exception)
    {
        var traceId = context.TraceIdentifier;
        _logger.LogError(exception, "未处理的 API 异常。追踪标识：{TraceId}", traceId);

        var (statusCode, title, detail) = exception switch
        {
            ArgumentException => (HttpStatusCode.BadRequest, "Bad Request", exception.Message),
            KeyNotFoundException => (HttpStatusCode.NotFound, "Not Found", exception.Message),
            UnauthorizedAccessException => (HttpStatusCode.Unauthorized, "Unauthorized", exception.Message),
            InvalidOperationException => (HttpStatusCode.Conflict, "Conflict", exception.Message),
            _ => (HttpStatusCode.InternalServerError, "Internal Server Error", "An unexpected error occurred.")
        };

        var response = ErrorResponse.Create((int)statusCode, title, detail, traceId);
        context.Response.StatusCode = (int)statusCode;
        context.Response.ContentType = "application/json";
        await context.Response.WriteAsJsonAsync(response.ToApiResponse());
    }
}
