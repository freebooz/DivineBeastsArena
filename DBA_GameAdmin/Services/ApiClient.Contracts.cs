/*
中文阅读说明：
- 所属应用：DBA_GameAdmin GM 管理后台。
- 文件职责：集中定义后台页面使用的 Game.Api DTO 契约。
- 阅读重点：类型命名应与后端 Admin/LiveOps/Platform 响应保持一一对应。
- 修改提示：新增 DTO 时优先复用 Game.Shared 契约；仅在后台展示需要裁剪字段时保留本地 DTO。
*/

namespace GameAdmin.Services;

public record AdminLoginRequestDto(string Username, string Password);
public record AdminLoginDto(string AccessToken, Guid AdminId, string Username, string Role);
public record AdminPlayerListDto(IReadOnlyList<PlayerDto> Items, int TotalCount, int Page, int PageSize);
public record PlayerDto(
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
public record PlayerDetailDto(
    Guid PlayerId,
    string Nickname,
    string? Email,
    string AccountType,
    string Status,
    int Level,
    long Exp,
    DateTimeOffset? LastLoginAt,
    PlayerStatisticsDto Statistics,
    IReadOnlyList<PlayerInventoryItemDto> Inventory,
    IReadOnlyList<PlayerUnlockDto> Unlocks);
public record PlayerStatisticsDto(
    int TotalMatches,
    int Wins,
    int Losses,
    int Draws,
    int Kills,
    int Deaths,
    int Assists,
    long Score,
    long PlayTimeSeconds);
public record PlayerInventoryItemDto(Guid Id, string ItemId, long Quantity, DateTimeOffset? ExpiresAt);
public record PlayerUnlockDto(string UnlockType, string UnlockId, string Source, DateTimeOffset CreatedAt);
public record AdminGameServerListDto(IReadOnlyList<GameServerDto> Items, int TotalCount, int Page, int PageSize);
public record GameServerDto(
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
public record AdminMatchListDto(IReadOnlyList<MatchDto> Items, int TotalCount, int Page, int PageSize);
public record MatchDto(
    Guid Id,
    Guid SessionId,
    string Mode,
    string MapId,
    int DurationSeconds,
    int PlayerCount,
    string ResultJson,
    DateTimeOffset CreatedAt);
public record AdminMatchDetailDto(
    Guid Id,
    Guid SessionId,
    string Mode,
    string MapId,
    int DurationSeconds,
    string ResultJson,
    DateTimeOffset CreatedAt,
    IReadOnlyList<MatchPlayerDto> Players);
public record MatchPlayerDto(
    Guid PlayerId,
    string? Team,
    string Result,
    int Kills,
    int Deaths,
    int Assists,
    int Score,
    long ExpDelta);
public record ConfigDto(
    Guid Id,
    string ConfigKey,
    string Version,
    string ContentJson,
    string Status,
    string Checksum,
    string Channel,
    string Region,
    string? MinClientVersion,
    string? MaxClientVersion,
    DateTimeOffset CreatedAt,
    DateTimeOffset? PublishedAt);
public record RoomDto(Guid Id, string Mode, string MapId, string Status, int PlayerCount);
public record AdminClientVersionListDto(IReadOnlyList<AdminClientVersionDto> Items, int TotalCount, int Page, int PageSize);
public record AdminClientVersionDto(
    Guid Id,
    string Version,
    string Channel,
    string Platform,
    string DownloadUrl,
    string Checksum,
    long SizeBytes,
    bool IsMandatory,
    bool IsActive,
    string? MinOsVersion,
    string? ReleaseNotes,
    DateTimeOffset CreatedAt);
public record UpsertClientVersionDto(
    string Version,
    string Channel,
    string Platform,
    string DownloadUrl,
    string Checksum,
    long SizeBytes,
    bool IsMandatory,
    bool IsActive,
    string? MinOsVersion,
    string? ReleaseNotes,
    string Reason);
public record AdminAuditLogListDto(IReadOnlyList<AdminAuditLogDto> Items, int TotalCount, int Page, int PageSize);
public record AdminAuditLogDto(
    Guid Id,
    Guid? AdminUserId,
    string? AdminUsername,
    string Action,
    string TargetType,
    string? TargetId,
    string? Reason,
    string? IpAddress,
    DateTimeOffset CreatedAt);
public record AdminInventoryLogDto(
    Guid Id,
    Guid PlayerId,
    string ItemId,
    long QuantityDelta,
    string Reason,
    DateTimeOffset CreatedAt);
public record AdminInventoryMutationRequest(Guid PlayerId, string ItemId, long Quantity, string Reason);
public record AdminReasonRequest(string Reason);
public record AdminFeedbackListDto(IReadOnlyList<AdminFeedbackDto> Items, int TotalCount, int Page, int PageSize);
public record AdminFeedbackDto(
    Guid Id,
    Guid? PlayerId,
    string? Nickname,
    string? Email,
    string FeedbackType,
    string? Title,
    string Status,
    DateTimeOffset CreatedAt,
    DateTimeOffset? UpdatedAt);
public record AdminSupportTicketListDto(IReadOnlyList<AdminSupportTicketDto> Items, int TotalCount, int Page, int PageSize);
public record AdminSupportTicketDto(
    Guid Id,
    Guid? PlayerId,
    string? Nickname,
    string TicketType,
    string Subject,
    string Status,
    string Priority,
    DateTimeOffset CreatedAt,
    DateTimeOffset? UpdatedAt);
public record PlatformApplicationsDto(DateTimeOffset GeneratedAt, IReadOnlyList<PlatformApplicationDto> Applications);
public record PlatformApplicationDto(
    string Id,
    string Name,
    string Category,
    string Directory,
    string Runtime,
    string Goal,
    string Status,
    string RunCommand,
    string HealthCheck,
    IReadOnlyList<string> Responsibilities,
    IReadOnlyList<string> IntegrationPoints,
    IReadOnlyList<string> NextSteps);

public record LiveOpsStatusDto(
    DateTimeOffset GeneratedAt,
    int TotalAccounts,
    int TotalPlayers,
    int TotalCharacters,
    int ActiveGameServers,
    int StaleGameServers,
    int OpenSupportTickets,
    int OpenReports,
    int ActiveAnnouncements,
    int ActiveEvents,
    string LatestClientVersion,
    IReadOnlyList<LiveOpsHealthItemDto> HealthItems);

public record LiveOpsHealthItemDto(string Name, string Status, string Detail);
