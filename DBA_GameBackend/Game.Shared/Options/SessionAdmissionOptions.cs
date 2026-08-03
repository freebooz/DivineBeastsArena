/*
中文阅读说明：
- 所属应用：DBA_GameBackend 共享配置层。
- 文件职责：集中配置会话连接凭证、重连凭证和匹配会话默认参数。
- 安全约束：有效期和令牌长度由环境配置覆盖，不在 API 服务中写死。
*/

namespace Game.Shared.Options;

public sealed class SessionAdmissionOptions
{
    public const string Section = "SessionAdmission";

    public int TokenByteLength { get; init; }
    public int ConnectionTokenLifetimeMinutes { get; init; }
    public int ReconnectTokenLifetimeMinutes { get; init; }
    public int ProvisionalTokenLifetimeMinutes { get; init; }
    public string MatchMapId { get; init; } = string.Empty;
    public int MatchMaxPlayers { get; init; }
}
