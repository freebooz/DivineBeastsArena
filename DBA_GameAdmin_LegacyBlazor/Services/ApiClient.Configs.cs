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
    // Configs
    public async Task<List<ConfigDto>?> GetConfigsAsync()
    {
        return await GetAsync<List<ConfigDto>>("/api/admin/configs");
    }

    public async Task<AdminClientVersionListDto?> GetClientVersionsAsync(int page = 1, int pageSize = 20)
    {
        return await GetAsync<AdminClientVersionListDto>($"/api/admin/client-versions?page={page}&pageSize={pageSize}");
    }

    public async Task<AdminClientVersionDto?> UpsertClientVersionAsync(UpsertClientVersionDto request)
    {
        return await PostAsync<AdminClientVersionDto, UpsertClientVersionDto>("/api/admin/client-versions", request);
    }

    // Rooms
    public async Task<List<RoomDto>?> GetRoomsAsync()
    {
        return await GetAsync<List<RoomDto>>("/api/rooms");
    }
}
