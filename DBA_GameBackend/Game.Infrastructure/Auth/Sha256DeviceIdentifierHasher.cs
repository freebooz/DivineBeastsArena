/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：以 SHA-256 哈希设备标识，避免持久化原始设备标识。
*/

using System.Security.Cryptography;
using System.Text;
using Game.Application.Auth;

namespace Game.Infrastructure.Auth;

public sealed class Sha256DeviceIdentifierHasher : IDeviceIdentifierHasher
{
    public string Hash(string deviceIdentifier)
    {
        var bytes = SHA256.HashData(Encoding.UTF8.GetBytes(deviceIdentifier));
        return Convert.ToHexString(bytes).ToLowerInvariant();
    }
}
