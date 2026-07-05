/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Microsoft.AspNetCore.Mvc;
using Game.Shared.Common;

namespace Game.Api.Extensions;

public static class EndpointResultsExtensions
{
    /// <summary>
    /// 统一返回格式 - 成功响应 / Unified response format - Success
    /// </summary>
    public static IResult ToApiResponse<T>(this ApiResponse<T> response) =>
        Results.Ok(response);

    /// <summary>
    /// 统一返回格式 - 错误响应（与成功响应格式一致）
    /// 使用 ApiResponse 格式返回错误，保持格式统一
    /// </summary>
    public static IResult ToProblem(this Game.Shared.Common.ErrorResponse error) =>
        Results.Json(error.ToApiResponse(), statusCode: error.Status);

    public static IResult ToValidationProblem(this Game.Shared.Common.ErrorResponse error) =>
        Results.Json(error.ToApiResponse(), statusCode: error.Status);
}

public static class ApiResponseExtensions
{
    /// <summary>
    /// 成功响应 / Success response
    /// </summary>
    public static IResult ToSuccess<T>(this T data, string? message = null) =>
        Results.Ok(ApiResponse<T>.Ok(data, message));

    /// <summary>
    /// 空成功响应 / Empty success response
    /// </summary>
    public static IResult ToSuccess(string? message = null) =>
        Results.Ok(ApiResponse.Ok(message));
}
