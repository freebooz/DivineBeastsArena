/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义跨进程/跨项目传输 DTO，客户端、后台和服务端都应以这里的字段契约为准。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Common;
using Game.Shared.Contracts.Character;

namespace Game.Shared.Contracts.Session;

public record SessionResponse(
    Guid Id,
    string SourceType,
    string Mode,
    string MapId,
    string Region,
    string Status,
    string? ServerIp,
    int? ServerPort,
    int MaxPlayers,
    DateTimeOffset CreatedAt,
    DateTimeOffset? StartedAt);

public record SessionConnectionResponse(
    Guid SessionId,
    string ServerIp,
    int ServerPort,
    string JoinTicket,
    DateTimeOffset JoinTicketExpiresAt,
    Guid PlayerId,
    Guid CharacterId,
    Guid ServerInstanceId,
    string BuildId,
    int TeamId,
    CharacterBuildSummaryDto? CharacterBuildSummary = null,
    string? ReconnectToken = null,
    DateTimeOffset? ReconnectTokenExpiresAt = null)
{
    // 兼容旧客户端；新客户端只读取 JoinTicket。
    public string SessionToken => JoinTicket;
    public DateTimeOffset TokenExpiresAt => JoinTicketExpiresAt;
    public string PlayerSessionToken => JoinTicket;
}

public record VillageAllocationRequest(Guid CharacterId);
public record VillageAllocationResponse(Guid SessionId, string Status);

/// <summary>
/// 前台“进入游戏”意图。客户端只提交已选择角色和其所属区服；
/// Dedicated Server 实例、连接地址和一次性票据均由服务端决定。
/// </summary>
public record GameEnterRequest(Guid CharacterId, Guid ServerId);

/// <summary>
/// 前台进入游戏响应。<c>Status</c> 为 <c>PENDING</c> 时表示 Dedicated Server 仍在分配或启动，
/// 客户端可通过同一意图安全重试；为 <c>READY</c> 时才会携带一次性 <c>GameTicket</c>。
/// AccessToken 绝不复用于本票据，也不得被写入本响应。
/// </summary>
public record GameEnterResponse(
    string Status,
    Guid SessionId,
    SessionConnectionResponse? Connection = null);

public record ReconnectTokenResponse(string ReconnectToken, DateTimeOffset ExpiresAt);

public record InternalCreateSessionFromRoomRequest(Guid RoomId);
public record InternalCreateSessionFromMatchRequest(Guid TicketId);
public record InternalAllocateServerRequest(Guid SessionId, string Ip, int Port, string RuntimeToken);
public record InternalMarkInProgressRequest(Guid SessionId);
public record InternalMarkCompletedRequest(Guid SessionId);
public record InternalMarkFailedRequest(Guid SessionId, string Reason);
