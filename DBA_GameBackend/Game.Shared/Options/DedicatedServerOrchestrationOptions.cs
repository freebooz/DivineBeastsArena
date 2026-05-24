/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载 Dedicated Server 编排层的启动、端口、镜像/可执行文件和超时配置。
- 阅读重点：Section 仍指向历史配置节 GameServerManager，用于兼容现有环境变量和 docker-compose。
- 修改提示：如需改配置节名，先提供迁移期双读逻辑，避免破坏线上环境变量。
*/

namespace Game.Shared.Options;

public sealed class DedicatedServerOrchestrationOptions
{
    // Keep the legacy section name for environment-variable and compose compatibility.
    public const string Section = "GameServerManager";

    public string ServerMode { get; init; } = "LocalProcess";
    public string PublicIp { get; init; } = "127.0.0.1";
    public int PortRangeStart { get; init; } = 7777;
    public int PortRangeEnd { get; init; } = 7797;
    public string UeServerImage { get; init; } = "divine-beasts-arena-server:latest";
    public string UeServerExecutablePath { get; init; } = string.Empty;
    public string BackendUrl { get; init; } = "http://localhost:8080";
    public bool AllowMockServerAllocation { get; init; } = true;
    public int StartupTimeoutSeconds { get; init; } = 120;
    public int HeartbeatTimeoutSeconds { get; init; } = 60;
    public int IdleTimeoutSeconds { get; init; } = 300;
    public int MaxServersPerMachine { get; init; } = 8;
}

