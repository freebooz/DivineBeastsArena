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
    public async Task<PlatformApplicationsDto?> GetPlatformApplicationsAsync()
    {
        return await GetAsync<PlatformApplicationsDto>("/api/platform/applications");
    }

    public async Task<LiveOpsStatusDto?> GetLiveOpsStatusAsync()
    {
        return await GetAsync<LiveOpsStatusDto>("/api/live-ops/status");
    }
}
