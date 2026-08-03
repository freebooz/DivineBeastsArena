/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：使用安全随机数和 SHA-256 实现会话凭证签发端口。
- 安全约束：不记录明文凭证；有效期和随机字节长度全部来自已校验配置。
*/

using System.Security.Cryptography;
using System.Text;
using Game.Application.Sessions;
using Game.Shared.Options;

namespace Game.Infrastructure.Security;

public sealed class CryptographicSessionCredentialIssuer(
    SessionAdmissionOptions options,
    TimeProvider timeProvider) : ISessionCredentialIssuer
{
    public IssuedSessionCredential IssueConnectionCredential()
    {
        return Issue(TimeSpan.FromMinutes(options.ConnectionTokenLifetimeMinutes));
    }

    public IssuedSessionCredential IssueReconnectCredential()
    {
        return Issue(TimeSpan.FromMinutes(options.ReconnectTokenLifetimeMinutes));
    }

    public IssuedSessionCredential IssueProvisionalCredential()
    {
        return Issue(TimeSpan.FromMinutes(options.ProvisionalTokenLifetimeMinutes));
    }

    private IssuedSessionCredential Issue(TimeSpan lifetime)
    {
        var plaintext = Convert.ToBase64String(RandomNumberGenerator.GetBytes(options.TokenByteLength));
        var hash = Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(plaintext))).ToLowerInvariant();
        return new IssuedSessionCredential(plaintext, hash, timeProvider.GetUtcNow().Add(lifetime));
    }
}
