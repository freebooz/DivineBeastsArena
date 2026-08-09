/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Shared.Common;

public sealed class ErrorResponse
{
    public string Type { get; init; } = "https://tools.ietf.org/html/rfc7807";
    public string Title { get; init; } = string.Empty;
    public string Detail { get; init; } = string.Empty;
    public int Status { get; init; }
    public string Code { get; init; } = "REQUEST_FAILED";
    public string? TraceId { get; init; }
    public Dictionary<string, object[]>? Errors { get; init; }

    /// <summary>
    /// 统一错误响应格式 / Unified error response format
    /// 将错误转换为 ApiResponse 格式以保持与成功响应格式一致
    /// </summary>
    public ApiResponse<object> ToApiResponse() => new()
    {
        Data = null,
        Success = false,
        Code = this.Code,
        Message = $"{Status}|{Title}|{Detail}",
        Timestamp = DateTimeOffset.UtcNow
    };

    public static ErrorResponse Create(
        int status,
        string title,
        string detail,
        string? traceId = null,
        string? code = null) => new()
    {
        Status = status,
        Code = code ?? $"HTTP_{status}",
        Title = title,
        Detail = detail,
        TraceId = traceId
    };

    public static ErrorResponse BadRequest(string detail, string? traceId = null) =>
        Create(400, "Bad Request", detail, traceId);

    public static ErrorResponse Unauthorized(string detail = "Unauthorized", string? traceId = null) =>
        Create(401, "Unauthorized", detail, traceId);

    public static ErrorResponse Forbidden(string detail = "Forbidden", string? traceId = null) =>
        Create(403, "Forbidden", detail, traceId);

    public static ErrorResponse NotFound(string detail, string? traceId = null) =>
        Create(404, "Not Found", detail, traceId);

    public static ErrorResponse Conflict(string detail, string? traceId = null) =>
        Create(409, "Conflict", detail, traceId);

    public static ErrorResponse Internal(string detail, string? traceId = null) =>
        Create(500, "Internal Server Error", detail, traceId);

    public static ErrorResponse Validation(Dictionary<string, object[]> errors, string? traceId = null) => new()
    {
        Status = 422,
        Title = "Validation Error",
        Detail = "One or more validation errors occurred.",
        TraceId = traceId,
        Errors = errors
    };
}
