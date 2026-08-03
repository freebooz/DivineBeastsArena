/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义跨进程/跨项目传输 DTO，客户端、后台和服务端都应以这里的字段契约为准。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System.Text.Json.Serialization;
using Game.Shared.Common;

namespace Game.Shared.Contracts.Match;

public record CreateMatchmakingTicketRequest(string Mode, string Region, int Mmr = 1000);

/// <summary>
/// 匹配票据响应。SessionId 与 MatchedSessionId 同值，兼容 UE 客户端只解析 sessionId 的路径。
/// </summary>
public record MatchmakingTicketResponse(
    Guid Id,
    Guid PlayerId,
    string Mode,
    string Region,
    int Mmr,
    string Status,
    Guid? MatchedSessionId,
    DateTimeOffset CreatedAt,
    DateTimeOffset? TimeoutAt)
{
    /// <summary>与 MatchedSessionId 同值，供客户端按 sessionId 字段读取。</summary>
    [JsonPropertyName("sessionId")]
    public Guid? SessionId => MatchedSessionId;
}
