/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：定义会话凭证签发端口及签发结果，不依赖具体密码学实现。
- 安全约束：明文凭证只返回给调用方一次，持久化层只能保存 Hash。
*/

namespace Game.Application.Sessions;

public sealed record IssuedSessionCredential(
    string Plaintext,
    string Hash,
    DateTimeOffset ExpiresAt);

public interface ISessionCredentialIssuer
{
    IssuedSessionCredential IssueConnectionCredential();
    IssuedSessionCredential IssueReconnectCredential();
    IssuedSessionCredential IssueProvisionalCredential();
}
