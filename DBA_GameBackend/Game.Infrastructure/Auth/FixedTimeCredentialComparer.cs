/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：使用固定时间比较敏感凭据，降低时序侧信道风险。
*/

using System.Security.Cryptography;
using System.Text;
using Game.Application.Auth;

namespace Game.Infrastructure.Auth;

public sealed class FixedTimeCredentialComparer : ISecureCredentialComparer
{
    public bool Equals(string? suppliedValue, string configuredValue)
    {
        if (string.IsNullOrWhiteSpace(suppliedValue))
            return false;

        var suppliedBytes = Encoding.UTF8.GetBytes(suppliedValue);
        var configuredBytes = Encoding.UTF8.GetBytes(configuredValue);
        return suppliedBytes.Length == configuredBytes.Length &&
            CryptographicOperations.FixedTimeEquals(suppliedBytes, configuredBytes);
    }
}
