/*
中文阅读说明：
- 所属应用：DBA_GameAdmin GM 管理后台。
- 文件职责：ApiClient 核心 HTTP、认证头和 API envelope 解析逻辑。
- 阅读重点：领域 API 方法位于 ApiClient.*.cs partial 文件，DTO 契约位于 ApiClient.Contracts.cs。
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
