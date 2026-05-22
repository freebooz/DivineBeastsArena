/*
中文阅读说明：
- 所属应用：GameAdmin GM 管理后台。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System.Net.Http.Headers;
using System.Net.Http.Json;
using System.Text.Json;
using Microsoft.Extensions.Logging;

namespace GameAdmin.Services;

public class ApiClient
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

    // Health check
    public async Task<bool> IsHealthyAsync()
    {
        try
        {
            ApplyAuthorizationHeader();
            var response = await _http.GetAsync("/health/live");
            return response.IsSuccessStatusCode;
        }
        catch { return false; }
    }

    // Players
    public async Task<AdminPlayerListDto?> GetPlayersAsync(int page = 1, int pageSize = 50)
    {
        return await GetAsync<AdminPlayerListDto>($"/api/admin/players?page={page}&pageSize={pageSize}");
    }

    public async Task<PlayerDetailDto?> GetPlayerAsync(Guid playerId)
    {
        return await GetAsync<PlayerDetailDto>($"/api/admin/players/{playerId}");
    }

    // Game Servers
    public async Task<AdminGameServerListDto?> GetGameServersAsync(int page = 1, int pageSize = 50)
    {
        return await GetAsync<AdminGameServerListDto>($"/api/admin/servers?page={page}&pageSize={pageSize}");
    }

    // Configs
    public async Task<List<ConfigDto>?> GetConfigsAsync()
    {
        return await GetAsync<List<ConfigDto>>("/api/admin/configs");
    }

    // Rooms
    public async Task<List<RoomDto>?> GetRoomsAsync()
    {
        return await GetAsync<List<RoomDto>>("/api/rooms");
    }

    public async Task<AdminMatchListDto?> GetMatchesAsync(int page = 1, int pageSize = 50)
    {
        return await GetAsync<AdminMatchListDto>($"/api/admin/matches?page={page}&pageSize={pageSize}");
    }

    public async Task<AdminMatchDetailDto?> GetMatchAsync(Guid matchId)
    {
        return await GetAsync<AdminMatchDetailDto>($"/api/admin/matches/{matchId}");
    }

    public async Task<PlatformApplicationsDto?> GetPlatformApplicationsAsync()
    {
        return await GetAsync<PlatformApplicationsDto>("/api/platform/applications");
    }

    public async Task<OperationsStatusDto?> GetOperationsStatusAsync()
    {
        return await GetAsync<OperationsStatusDto>("/api/operations/status");
    }

    public async Task<AdminAuditLogListDto?> GetAuditLogsAsync(int page = 1, int pageSize = 50)
    {
        return await GetAsync<AdminAuditLogListDto>($"/api/admin/audit-logs?page={page}&pageSize={pageSize}");
    }

    public async Task<List<AdminInventoryLogDto>?> GetInventoryLogsAsync(int page = 1, int pageSize = 50)
    {
        return await GetAsync<List<AdminInventoryLogDto>>($"/api/admin/inventory/logs?page={page}&pageSize={pageSize}");
    }

    public async Task<bool> GrantInventoryItemAsync(Guid playerId, string itemId, long quantity, string reason)
    {
        return await PostVoidAsync("/api/admin/inventory/grant", new AdminInventoryMutationRequest(playerId, itemId, quantity, reason));
    }

    public async Task<bool> DeductInventoryItemAsync(Guid playerId, string itemId, long quantity, string reason)
    {
        return await PostVoidAsync("/api/admin/inventory/deduct", new AdminInventoryMutationRequest(playerId, itemId, quantity, reason));
    }

    public async Task<AdminFeedbackListDto?> GetFeedbackAsync(int page = 1, int pageSize = 50)
    {
        return await GetAsync<AdminFeedbackListDto>($"/api/admin/feedback?page={page}&pageSize={pageSize}");
    }

    public async Task<AdminSupportTicketListDto?> GetSupportTicketsAsync(int page = 1, int pageSize = 50)
    {
        return await GetAsync<AdminSupportTicketListDto>($"/api/admin/support/tickets?page={page}&pageSize={pageSize}");
    }

    private void ApplyAuthorizationHeader()
    {
        _http.DefaultRequestHeaders.Authorization = string.IsNullOrWhiteSpace(_authState.AccessToken)
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

public record OperationsStatusDto(
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
    IReadOnlyList<OperationsHealthItemDto> HealthItems);

public record OperationsHealthItemDto(string Name, string Status, string Detail);
