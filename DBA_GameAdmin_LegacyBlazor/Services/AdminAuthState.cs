/*
中文阅读说明：
- 所属应用：GameAdmin GM 管理后台。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace GameAdmin.Services;

using System.IdentityModel.Tokens.Jwt;

public sealed class AdminAuthState
{
    public string? AccessToken { get; private set; }
    public Guid? AdminId { get; private set; }
    public string? Username { get; private set; }
    public string? Role { get; private set; }
    public DateTimeOffset? ExpiresAt { get; private set; }

    public bool IsAuthenticated => !string.IsNullOrWhiteSpace(AccessToken) && !IsExpired;

    public bool IsExpired => ExpiresAt.HasValue && ExpiresAt.Value <= DateTimeOffset.UtcNow;

    public void SignIn(AdminLoginDto login)
    {
        AccessToken = login.AccessToken;
        AdminId = login.AdminId;
        Username = login.Username;
        Role = login.Role;
        ExpiresAt = ReadTokenExpiry(login.AccessToken);
    }

    public void SignOut()
    {
        AccessToken = null;
        AdminId = null;
        Username = null;
        Role = null;
        ExpiresAt = null;
    }

    private static DateTimeOffset? ReadTokenExpiry(string accessToken)
    {
        try
        {
            var token = new JwtSecurityTokenHandler().ReadJwtToken(accessToken);
            return token.ValidTo == DateTime.MinValue
                ? null
                : new DateTimeOffset(DateTime.SpecifyKind(token.ValidTo, DateTimeKind.Utc));
        }
        catch
        {
            return null;
        }
    }
}
