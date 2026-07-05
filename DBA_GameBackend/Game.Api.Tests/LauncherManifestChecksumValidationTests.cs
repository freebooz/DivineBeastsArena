/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端测试。
- 文件职责：验证启动器发布清单的校验和格式，避免 Admin 写入启动器无法接受的版本元数据。
- 阅读重点：这里测试的是发布元数据合同，不依赖数据库或 HTTP 服务。
*/

using Game.Api.Endpoints.Admin;

namespace Game.Api.Tests;

public class LauncherManifestChecksumValidationTests
{
    [Fact]
    public void IsValidLauncherManifestChecksum_WhenSha256Hex_ReturnsTrue()
    {
        var checksum = new string('a', 64);

        var result = AdminEndpoints.IsValidLauncherManifestChecksum(checksum);

        Assert.True(result);
    }

    [Fact]
    public void IsValidLauncherManifestChecksum_WhenPlaceholder_ReturnsFalse()
    {
        var result = AdminEndpoints.IsValidLauncherManifestChecksum("dev-checksum-placeholder");

        Assert.False(result);
    }
}
