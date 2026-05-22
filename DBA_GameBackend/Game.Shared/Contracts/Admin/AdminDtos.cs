/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义跨进程/跨项目传输 DTO，客户端、后台和服务端都应以这里的字段契约为准。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Common;

namespace Game.Shared.Contracts.Admin;

public record AdminLoginRequest(string Username, string Password);
public record AdminLoginResponse(string AccessToken, Guid AdminId, string Username, string Role);

public record AdminProfileResponse(Guid AdminId, string Username, string Role, DateTimeOffset? LastLoginAt);

public record AdminPlayerListResponse(
    IReadOnlyList<AdminPlayerListItem> Items,
    int TotalCount,
    int Page,
    int PageSize);

public record AdminPlayerListItem(
    Guid PlayerId,
    string Nickname,
    Guid? AccountId,
    string AccountType,
    string? Email,
    string AccountStatus,
    int Level,
    long Exp,
    DateTimeOffset CreatedAt,
    DateTimeOffset? LastLoginAt,
    int CharacterCount,
    string? SelectedCharacterName);

public record AdminAuditLogListResponse(
    IReadOnlyList<AdminAuditLogItem> Items,
    int TotalCount,
    int Page,
    int PageSize);

public record AdminAuditLogItem(
    Guid Id,
    Guid? AdminUserId,
    string? AdminUsername,
    string Action,
    string TargetType,
    string? TargetId,
    string? Reason,
    string? IpAddress,
    DateTimeOffset CreatedAt);

public record AdminFeedbackListResponse(
    IReadOnlyList<AdminFeedbackItem> Items,
    int TotalCount,
    int Page,
    int PageSize);

public record AdminFeedbackItem(
    Guid Id,
    Guid? PlayerId,
    string? Nickname,
    string? Email,
    string FeedbackType,
    string? Title,
    string Status,
    DateTimeOffset CreatedAt,
    DateTimeOffset? UpdatedAt);

public record AdminSupportTicketListResponse(
    IReadOnlyList<AdminSupportTicketItem> Items,
    int TotalCount,
    int Page,
    int PageSize);

public record AdminSupportTicketItem(
    Guid Id,
    Guid? PlayerId,
    string? Nickname,
    string TicketType,
    string Subject,
    string Status,
    string Priority,
    DateTimeOffset CreatedAt,
    DateTimeOffset? UpdatedAt);

public record AdminGameServerListResponse(
    IReadOnlyList<AdminGameServerItem> Items,
    int TotalCount,
    int Page,
    int PageSize);

public record AdminGameServerItem(
    Guid Id,
    Guid? SessionId,
    string? Mode,
    string? MapId,
    string? Region,
    string? BuildVersion,
    string Ip,
    int Port,
    string Status,
    DateTimeOffset StartedAt,
    DateTimeOffset? LastHeartbeatAt,
    DateTimeOffset? EndedAt);

public record AdminMatchListResponse(
    IReadOnlyList<AdminMatchListItem> Items,
    int TotalCount,
    int Page,
    int PageSize);

public record AdminMatchListItem(
    Guid Id,
    Guid SessionId,
    string Mode,
    string MapId,
    int DurationSeconds,
    int PlayerCount,
    string ResultJson,
    DateTimeOffset CreatedAt);

public record AdminMatchDetailResponse(
    Guid Id,
    Guid SessionId,
    string Mode,
    string MapId,
    int DurationSeconds,
    string ResultJson,
    DateTimeOffset CreatedAt,
    IReadOnlyList<AdminMatchPlayerItem> Players);

public record AdminMatchPlayerItem(
    Guid PlayerId,
    string? Team,
    string Result,
    int Kills,
    int Deaths,
    int Assists,
    int Score,
    long ExpDelta);

public record PlayerDetailResponse(
    Guid PlayerId,
    string Nickname,
    string? Email,
    string AccountType,
    string Status,
    int Level,
    long Exp,
    DateTimeOffset? LastLoginAt,
    PlayerStatisticsDto Statistics,
    IReadOnlyList<InventoryItemDto> Inventory,
    IReadOnlyList<PlayerUnlockDto> Unlocks);

public record PlayerStatisticsDto(
    int TotalMatches, int Wins, int Losses, int Draws,
    int Kills, int Deaths, int Assists, long Score, long PlayTimeSeconds);

public record InventoryItemDto(Guid Id, string ItemId, long Quantity, DateTimeOffset? ExpiresAt);
public record PlayerUnlockDto(string UnlockType, string UnlockId, string Source, DateTimeOffset CreatedAt);

public record GrantItemRequest(Guid PlayerId, string ItemId, long Quantity, string Reason);
public record DeductItemRequest(Guid PlayerId, string ItemId, long Quantity, string Reason);

public record BanPlayerRequest(Guid AccountId, Guid? PlayerId, string Reason, int DurationDays);
public record UnbanPlayerRequest(Guid AccountId, string Reason);

public record ConfigAdminResponse(
    Guid Id, string ConfigKey, string Version, string Status,
    string Channel, string Region, DateTimeOffset CreatedAt, DateTimeOffset? PublishedAt);

public record GameServerResponse(
    Guid Id, Guid? SessionId, string? Mode, string Ip, int Port,
    string Status, DateTimeOffset StartedAt, DateTimeOffset? LastHeartbeatAt);

public record MatchAdminResponse(
    Guid Id, Guid SessionId, string Mode, string MapId,
    int DurationSeconds, string ResultJson, DateTimeOffset CreatedAt);

public record AuditLogResponse(
    Guid Id, string Action, string TargetType, string? TargetId,
    string? Reason, string? IpAddress, DateTimeOffset CreatedAt);
