/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：通过 EF Core 实现当前登录账号身份的只读查询端口。
*/

using Game.Application.Auth;
using Game.Shared.Contracts.Auth;
using Microsoft.EntityFrameworkCore;

namespace Game.Infrastructure.Database.Auth;

public sealed class EfAuthenticatedAccountQueryStore(GameDbContext db)
    : IAuthenticatedAccountQueryStore
{
    public async Task<MeResponse?> FindByAccountIdAsync(
        Guid accountId,
        CancellationToken cancellationToken = default)
    {
        return await db.Accounts
            .AsNoTracking()
            .Where(x => x.Id == accountId && x.PlayerIdentity != null)
            .Select(x => new MeResponse(
                x.Id,
                x.PlayerIdentity!.PlayerId,
                // /me 面向游戏客户端优先返回玩家档案名；极早期异常数据若尚未补齐档案，
                // 临时回退账号身份显示名以保持只读接口可用，首次认证会负责补齐正式游戏名。
                x.PlayerIdentity.PlayerProfile != null
                    ? x.PlayerIdentity.PlayerProfile.Nickname
                    : x.PlayerIdentity.DisplayName,
                x.AccountType,
                x.Email))
            .FirstOrDefaultAsync(cancellationToken);
    }
}
