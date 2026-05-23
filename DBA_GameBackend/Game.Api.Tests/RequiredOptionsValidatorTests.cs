/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端测试。
- 文件职责：验证启动期必需配置校验规则，避免生产环境缺失关键配置。
- 阅读重点：每个测试只覆盖一个高风险配置段。
- 修改提示：新增 RequiredOptionsValidator 规则时同步补充这里的失败/成功样例。
*/

using Game.Shared.Options;

namespace Game.Api.Tests;

public class RequiredOptionsValidatorTests
{
    [Fact]
    public void ValidateInternalApiKey_WhenShort_ThrowsReadableConfigKey()
    {
        var ex = Assert.Throws<InvalidOperationException>(() =>
            RequiredOptionsValidator.ValidateInternalApiKey("short"));

        Assert.Contains("InternalApi:Key", ex.Message);
    }

    [Fact]
    public void ValidateDedicatedServerOrchestration_WhenPortRangeInvalid_ThrowsReadableConfigKey()
    {
        var options = new DedicatedServerOrchestrationOptions
        {
            ServerMode = "LocalProcess",
            PublicIp = "127.0.0.1",
            PortRangeStart = 8000,
            PortRangeEnd = 7777,
            MaxServersPerMachine = 1,
            StartupTimeoutSeconds = 120,
            HeartbeatTimeoutSeconds = 60,
            IdleTimeoutSeconds = 300
        };

        var ex = Assert.Throws<InvalidOperationException>(() =>
            RequiredOptionsValidator.ValidateDedicatedServerOrchestration(options));

        Assert.Contains("GameServerManager", ex.Message);
    }

    [Fact]
    public void ValidateDedicatedServerOrchestration_WhenDockerModeWithoutImage_ThrowsReadableConfigKey()
    {
        var options = new DedicatedServerOrchestrationOptions
        {
            ServerMode = "Docker",
            PublicIp = "127.0.0.1",
            PortRangeStart = 7777,
            PortRangeEnd = 8000,
            UeServerImage = "",
            MaxServersPerMachine = 1,
            StartupTimeoutSeconds = 120,
            HeartbeatTimeoutSeconds = 60,
            IdleTimeoutSeconds = 300
        };

        var ex = Assert.Throws<InvalidOperationException>(() =>
            RequiredOptionsValidator.ValidateDedicatedServerOrchestration(options));

        Assert.Contains("GameServerManager:UeServerImage", ex.Message);
    }

    [Fact]
    public void ValidateAll_WhenOptionsAreComplete_DoesNotThrow()
    {
        RequiredOptionsValidator.ValidateDatabase(new DatabaseOptions
        {
            ConnectionString = "Host=localhost;Database=game;Username=postgres;Password=postgres"
        });
        RequiredOptionsValidator.ValidateRedis(new RedisOptions { ConnectionString = "localhost:6379" });
        RequiredOptionsValidator.ValidateJwt(new JwtOptions
        {
            Secret = "TEST-SECRET-MINIMUM-32-CHARACTERS",
            Issuer = "GameApi",
            Audience = "GameClients"
        });
        RequiredOptionsValidator.ValidateInternalApiKey("TEST-INTERNAL-API-KEY-MIN-32-CHARS");
        RequiredOptionsValidator.ValidateDedicatedServerOrchestration(new DedicatedServerOrchestrationOptions
        {
            ServerMode = "LocalProcess",
            PublicIp = "127.0.0.1",
            PortRangeStart = 7777,
            PortRangeEnd = 8000,
            MaxServersPerMachine = 4,
            StartupTimeoutSeconds = 120,
            HeartbeatTimeoutSeconds = 60,
            IdleTimeoutSeconds = 300
        });
    }
}

