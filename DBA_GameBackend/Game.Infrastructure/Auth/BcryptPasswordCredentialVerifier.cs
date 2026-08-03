/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：实现应用层密码凭据校验端口，封装 BCrypt 依赖。
*/

using Game.Application.Auth;

namespace Game.Infrastructure.Auth;

public sealed class BcryptPasswordCredentialVerifier : IPasswordCredentialVerifier
{
    public string Hash(string password) => PasswordHasher.Hash(password);

    public bool Verify(string password, string passwordHash) =>
        PasswordHasher.Verify(password, passwordHash);
}
