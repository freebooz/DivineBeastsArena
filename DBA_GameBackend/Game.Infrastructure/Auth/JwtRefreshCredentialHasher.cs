/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：复用 JWT 服务的安全哈希实现处理刷新令牌。
*/

using Game.Application.Auth;

namespace Game.Infrastructure.Auth;

public sealed class JwtRefreshCredentialHasher(IJwtTokenService jwtTokenService)
    : IRefreshCredentialHasher
{
    public string Hash(string refreshToken) => jwtTokenService.HashToken(refreshToken);
}
