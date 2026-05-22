/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义跨进程/跨项目传输 DTO，客户端、后台和服务端都应以这里的字段契约为准。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Shared.Contracts.Runtime;

public record AllocateServerRequest(string Mode, string MapId, string Region, string BuildVersion, string Ip, int Port);

public record HeartbeatRequest(int? CurrentPlayers);

public record StoppedServerRequest(string? ExitCode, string? CrashReason);

public record ServerInfoResponse(
    Guid ServerId,
    string Ip,
    int Port,
    string RuntimeToken,
    DateTimeOffset TokenExpiresAt);