/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端测试。
- 文件职责：验证外部 UE evidence runner 托管 Dedicated Server 时，后端只负责分配端口和 runtime token。
- 阅读重点：External mode 不是 mock；它保留生产 token/状态，但不会启动本地进程。
- 修改提示：调整 Dedicated Server 编排模式时，请同步覆盖这里的分配与事件断言。
*/

using Game.Infrastructure.Database;
using Game.ServerManagement.DedicatedServers;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore.Diagnostics;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging.Abstractions;
using Microsoft.Extensions.Options;

namespace Game.Api.Tests;

public class DedicatedServerOrchestratorExternalModeTests
{
    [Fact]
    public async Task AllocateAsync_WhenExternalMode_ReturnsRuntimeTokenAndDoesNotStartProcess()
    {
        await using var db = CreateDbContext();
        var sessionId = Guid.NewGuid();
        db.GameSessions.Add(new()
        {
            Id = sessionId,
            SourceType = "ROOM",
            SourceId = Guid.NewGuid(),
            Mode = "classic",
            MapId = "LobbyMap",
            Region = "local",
            Status = "CREATED",
            MaxPlayers = 2,
            CreatedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();

        var orchestrator = new DedicatedServerOrchestrator(
            db,
            Options.Create(new DedicatedServerOrchestrationOptions
            {
                ServerMode = "External",
                PublicIp = "127.0.0.1",
                PortRangeStart = 7777,
                PortRangeEnd = 7777,
                AllowMockServerAllocation = false,
                MaxServersPerMachine = 4,
                StartupTimeoutSeconds = 120,
                HeartbeatTimeoutSeconds = 60,
                IdleTimeoutSeconds = 300
            }),
            NullLogger<DedicatedServerOrchestrator>.Instance);

        var allocation = await orchestrator.AllocateAsync(new AllocateDedicatedServerCommand(
            sessionId,
            "classic",
            "LobbyMap",
            "local",
            "test-build"));

        var server = await db.GameServerInstances.SingleAsync(x => x.Id == allocation!.ServerId);
        var processEvents = await db.GameServerEvents.CountAsync(x => x.ServerId == allocation!.ServerId && x.EventType == "PROCESS_STARTED");
        var externalEvents = await db.GameServerEvents.CountAsync(x => x.ServerId == allocation!.ServerId && x.EventType == "LAUNCH_SKIPPED_EXTERNAL");

        Assert.NotNull(allocation);
        Assert.False(string.IsNullOrWhiteSpace(allocation!.RuntimeToken));
        Assert.Null(server.ProcessId);
        Assert.Equal("STARTING", server.Status);
        Assert.Equal(0, processEvents);
        Assert.Equal(1, externalEvents);
    }

    private static GameDbContext CreateDbContext()
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"dedicated-server-external-{Guid.NewGuid()}")
            .ConfigureWarnings(x => x.Ignore(InMemoryEventId.TransactionIgnoredWarning))
            .Options;
        return new GameDbContext(options);
    }
}
