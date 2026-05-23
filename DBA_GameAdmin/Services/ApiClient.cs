/*
中文阅读说明：
- 所属应用：GameAdmin GM 管理后台。
- 文件职责：ApiClient 核心 HTTP、认证头和 API envelope 解析逻辑。
- 阅读重点：领域 API 方法位于 ApiClient.*.cs partial 文件。
- 修改提示：保持这里聚焦通用通信能力，不直接堆叠业务页面方法。
*/

using System.Net.Http.Headers;
using System.Net.Http.Json;
using System.Text.Json;
using Microsoft.Extensions.Logging;

namespace GameAdmin.Services;
public partial class ApiClient
{
    private static readonly JsonSerializerOptions JsonOptions = new(JsonSerializerDefaults.Web)
    {
        PropertyNameCaseInsensitive = true
    };

    private readonly HttpClient _http;
    private readonly AdminAuthState _authState;
    private readonly ILogger<ApiClient> _logger;

    public ApiClient(HttpClient http, AdminAuthState authState, ILogger<ApiClient> logger)
    {
        _http = http;
        _authState = authState;
        _logger = logger;
    }

    public async Task<T?> GetAsync<T>(string url) where T : class
    {
        try
        {
            ApplyAuthorizationHeader();
            var response = await _http.GetAsync(url);
            if (!response.IsSuccessStatusCode)
            {
                _logger.LogWarning("GET {Url} failed with {StatusCode}", url, response.StatusCode);
                return null;
            }

            var payload = await response.Content.ReadAsStringAsync();
            return DeserializeApiPayload<T>(payload);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "GET {Url} failed", url);
            return null;
        }
    }

    public async Task<bool> PostVoidAsync<TBody>(string url, TBody body)
    {
        try
        {
            ApplyAuthorizationHeader();
            var response = await _http.PostAsJsonAsync(url, body);
            return response.IsSuccessStatusCode;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "POST {Url} failed", url);
            return false;
        }
    }

    public async Task<TResponse?> PostAsync<TResponse, TBody>(string url, TBody body)
        where TResponse : class
    {
        try
        {
            ApplyAuthorizationHeader();
            var response = await _http.PostAsJsonAsync(url, body);
            if (!response.IsSuccessStatusCode)
            {
                _logger.LogWarning("POST {Url} failed with {StatusCode}", url, response.StatusCode);
                return null;
            }

            var payload = await response.Content.ReadAsStringAsync();
            return DeserializeApiPayload<TResponse>(payload);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "POST {Url} failed", url);
            return null;
        }
    }

    public async Task<AdminLoginDto?> LoginAdminAsync(string username, string password)
    {
        try
        {
            _http.DefaultRequestHeaders.Authorization = null;
            var response = await _http.PostAsJsonAsync("/api/admin/auth/login", new AdminLoginRequestDto(username, password));
            if (!response.IsSuccessStatusCode)
            {
                _logger.LogWarning("Admin login failed with {StatusCode}", response.StatusCode);
                return null;
            }

            var payload = await response.Content.ReadAsStringAsync();
            return DeserializeApiPayload<AdminLoginDto>(payload);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Admin login failed");
            return null;
        }
    }

    private void ApplyAuthorizationHeader()
    {
        _http.DefaultRequestHeaders.Authorization = !_authState.IsAuthenticated || string.IsNullOrWhiteSpace(_authState.AccessToken)
            ? null
            : new AuthenticationHeaderValue("Bearer", _authState.AccessToken);
    }

    private static T? DeserializeApiPayload<T>(string payload) where T : class
    {
        if (string.IsNullOrWhiteSpace(payload))
        {
            return null;
        }

        using var document = JsonDocument.Parse(payload);
        var root = document.RootElement;

        if (root.ValueKind == JsonValueKind.Object &&
            root.TryGetProperty("success", out var successElement) &&
            root.TryGetProperty("data", out var dataElement))
        {
            if (successElement.ValueKind == JsonValueKind.False || dataElement.ValueKind == JsonValueKind.Null)
            {
                return null;
            }

            return dataElement.Deserialize<T>(JsonOptions);
        }

        return JsonSerializer.Deserialize<T>(payload, JsonOptions);
    }
}

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
