/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义跨进程/跨项目传输 DTO，客户端、后台和服务端都应以这里的字段契约为准。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Common;

namespace Game.Shared.Contracts.GameServer;

public record InternalAllocateServerResponse(Guid ServerId, string Ip, int Port, string RuntimeToken, DateTimeOffset TokenExpiresAt);

public record RuntimeRegisterRequest(Guid ServerId, Guid SessionId, string RuntimeToken);
public record RuntimeReadyRequest(Guid ServerId, Guid SessionId, string RuntimeToken);
public record RuntimeHeartbeatRequest(Guid ServerId, Guid SessionId, string RuntimeToken);
public record RuntimePlayerJoinedRequest(
    Guid ServerId,
    Guid SessionId,
    string RuntimeToken,
    Guid PlayerId,
    string? Team,
    int SlotIndex,
    string? PlayerSessionToken = null,
    string? Zodiac = null,
    string? PrimaryElement = null,
    string? FiveCamp = null,
    string? FixedSkillGroupId = null);
public record RuntimePlayerLeftRequest(Guid ServerId, Guid SessionId, string RuntimeToken, Guid PlayerId);
public record RuntimeMatchStartedRequest(Guid ServerId, Guid SessionId, string RuntimeToken);
public record RuntimeMatchEndedRequest(Guid ServerId, Guid SessionId, string RuntimeToken);
public record RuntimeMatchResultsRequest(
    Guid ServerId,
    Guid SessionId,
    string RuntimeToken,
    string IdempotencyKey,
    string ResultJson,
    IReadOnlyList<RuntimePlayerResultDto> Players);

public record RuntimePlayerResultDto(
    Guid PlayerId,
    string? Team,
    string Result,
    int Kills,
    int Deaths,
    int Assists,
    int Score,
    long ExpDelta,
    IReadOnlyDictionary<string, object> Rewards);

public record MatchResultsSubmitRequest(
    Guid SessionId,
    string IdempotencyKey,
    string ResultJson,
    IReadOnlyList<MatchResultPlayerDto> Players);

public record MatchResultPlayerDto(
    Guid PlayerId,
    string? Team,
    string Result,
    int Kills,
    int Deaths,
    int Assists,
    int Score,
    long ExpDelta,
    IReadOnlyDictionary<string, object> Rewards);

public record InternalGameServerResponse(
    Guid Id,
    Guid? SessionId,
    string? Mode,
    string? MapId,
    string Ip,
    int Port,
    string Status,
    DateTimeOffset StartedAt,
    DateTimeOffset? LastHeartbeatAt);
