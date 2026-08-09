/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Shared.Common;

public class ApiResponse<T>
{
    public T? Data { get; init; }
    public bool Success { get; init; } = true;
    /** 机器可读业务码；客户端必须基于该字段映射本地化文案。 */
    public string Code { get; init; } = "OK";
    public string? Message { get; init; }
    public DateTimeOffset Timestamp { get; init; } = DateTimeOffset.UtcNow;

    public static ApiResponse<T> Ok(T data, string? message = null) => new()
    {
        Data = data,
        Success = true,
        Code = "OK",
        Message = message
    };

    public static ApiResponse<T> Fail(string message, string code = "REQUEST_FAILED") => new()
    {
        Data = default,
        Success = false,
        Code = code,
        Message = message
    };
}

public class ApiResponse : ApiResponse<object>
{
    public static ApiResponse Ok(string? message = null) => new()
    {
        Success = true,
        Code = "OK",
        Message = message
    };

    public static ApiResponse Fail(string message) => new()
    {
        Success = false,
        Code = "REQUEST_FAILED",
        Message = message
    };
}
