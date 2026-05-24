/*
中文阅读说明：
- 所属应用：GameAdmin GM 管理后台。
- 文件职责：ApiClient partial 文件，按后台领域封装 Game.Api 调用。
- 阅读重点：核心 HTTP/envelope 逻辑在 ApiClient.cs，领域方法分布在 ApiClient.*.cs。
- 修改提示：新增后台页面接口时优先放到对应领域 partial 文件，避免 ApiClient.cs 膨胀。
*/

namespace GameAdmin.Services;
public partial class ApiClient
{
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
}
