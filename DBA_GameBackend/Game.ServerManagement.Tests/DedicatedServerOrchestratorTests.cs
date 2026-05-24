/*
中文阅读说明：
- 所属应用：DBA_GameBackend / Game.ServerManagement。
- 文件职责：测试 Dedicated Server 编排的关键状态机，覆盖端口分配、幂等、释放和超时维护。
- 阅读重点：LocalProcess 未配置可执行文件时会保持 mock STARTING 状态，便于在 CI 中验证分配流程。
- 修改提示：接入真实 UE Dedicated Server 或 Agones 后，保留这些基础幂等测试，再补外部进程/容器集成测试。
*/

using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.ServerManagement.DedicatedServers;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Diagnostics;
using Microsoft.Extensions.Logging.Abstractions;
using Microsoft.Extensions.Options;

namespace Game.ServerManagement.Tests;

public class DedicatedServerOrchestratorTests
{
    [Fact]
    public async Task AllocateAsync_CalledTwiceForSameSession_ReturnsSameServerAndDoesNotAllocateSecondPort()
    {
        await using var db = CreateDbContext();
        var service = CreateService(db);
        var sessionId = Guid.NewGuid();
        var command = new AllocateDedicatedServerCommand(sessionId, "classic", "arena_01", "cn", "test-build");

        var first = await service.AllocateAsync(command);
        var second = await service.AllocateAsync(command);

        Assert.NotNull(first);
        Assert.NotNull(second);
        Assert.Equal(first!.ServerId, second!.ServerId);
        Assert.False(string.IsNullOrWhiteSpace(first.RuntimeToken));
        Assert.Null(second.RuntimeToken);
        Assert.Equal(1, await db.GameServerInstances.CountAsync(x => x.SessionId == sessionId));
        Assert.Equal(1, await db.PortAllocations.CountAsync(x => x.Status == "ALLOCATED"));
    }

    [Fact]
    public async Task ReleaseAsync_StopsServerAndFreesAllocatedPort()
    {
        await using var db = CreateDbContext();
        var service = CreateService(db);
        var server = await service.AllocateAsync(new AllocateDedicatedServerCommand(Guid.NewGuid(), "classic", "arena_01", "cn", null));

        var released = await service.ReleaseAsync(server!.ServerId, "match completed");

        var stored = await db.GameServerInstances.SingleAsync(x => x.Id == server.ServerId);
        var port = await db.PortAllocations.SingleAsync(x => x.Port == server.Port);
        Assert.True(released);
        Assert.Equal("STOPPED", stored.Status);
        Assert.Equal("FREE", port.Status);
        Assert.Null(port.ServerId);
        Assert.Contains(await db.GameServerEvents.ToListAsync(), x => x.ServerId == server.ServerId && x.EventType == "RELEASED");
    }

    [Fact]
    public async Task RunMaintenanceAsync_WhenStartingTimedOut_MarksTimeoutAndFreesPort()
    {
        await using var db = CreateDbContext();
        var service = CreateService(db, new DedicatedServerOrchestrationOptions
        {
            ServerMode = "LocalProcess",
            PublicIp = "127.0.0.1",
            PortRangeStart = 7777,
            PortRangeEnd = 7778,
            StartupTimeoutSeconds = 30,
            HeartbeatTimeoutSeconds = 30,
            IdleTimeoutSeconds = 60,
            MaxServersPerMachine = 4
        });
        var serverId = Guid.NewGuid();
        db.GameServerInstances.Add(NewServer(serverId, "STARTING", 7777, startedAt: DateTimeOffset.UtcNow.AddMinutes(-5)));
        db.PortAllocations.Add(new PortAllocation
        {
            Port = 7777,
            Status = "ALLOCATED",
            ServerId = serverId,
            AllocatedAt = DateTimeOffset.UtcNow.AddMinutes(-5)
        });
        await db.SaveChangesAsync();

        var changed = await service.RunMaintenanceAsync();

        var stored = await db.GameServerInstances.SingleAsync(x => x.Id == serverId);
        var port = await db.PortAllocations.SingleAsync(x => x.Port == 7777);
        Assert.Equal(1, changed);
        Assert.Equal("TIMEOUT", stored.Status);
        Assert.Equal("FREE", port.Status);
        Assert.Contains(await db.GameServerEvents.ToListAsync(), x => x.ServerId == serverId && x.EventType == "MAINTENANCE_TIMEOUT");
    }

    [Fact]
    public async Task AllocateAsync_WhenExecutableMissingAndMockDisabled_MarksFailedAndFreesPort()
    {
        await using var db = CreateDbContext();
        var service = CreateService(db, new DedicatedServerOrchestrationOptions
        {
            ServerMode = "LocalProcess",
            PublicIp = "127.0.0.1",
            PortRangeStart = 7777,
            PortRangeEnd = 7778,
            UeServerExecutablePath = string.Empty,
            BackendUrl = "http://localhost:8080",
            AllowMockServerAllocation = false,
            StartupTimeoutSeconds = 30,
            HeartbeatTimeoutSeconds = 30,
            IdleTimeoutSeconds = 60,
            MaxServersPerMachine = 4
        });

        var allocated = await service.AllocateAsync(new AllocateDedicatedServerCommand(Guid.NewGuid(), "classic", "arena_01", "cn", null));

        Assert.NotNull(allocated);
        Assert.Equal("FAILED", allocated!.Status);
        var stored = await db.GameServerInstances.SingleAsync(x => x.Id == allocated.ServerId);
        var port = await db.PortAllocations.SingleAsync(x => x.Port == allocated.Port);
        Assert.Equal("FAILED", stored.Status);
        Assert.Equal("FREE", port.Status);
        Assert.Null(port.ServerId);
        Assert.Contains(await db.GameServerEvents.ToListAsync(), x => x.ServerId == allocated.ServerId && x.EventType == "LAUNCH_FAILED_CONFIG");
    }

    [Fact]
    public async Task RunMaintenanceAsync_WhenHeartbeatTimedOut_StopsServerAndFreesPort()
    {
        await using var db = CreateDbContext();
        var service = CreateService(db, new DedicatedServerOrchestrationOptions
        {
            ServerMode = "LocalProcess",
            PublicIp = "127.0.0.1",
            PortRangeStart = 7777,
            PortRangeEnd = 7778,
            StartupTimeoutSeconds = 30,
            HeartbeatTimeoutSeconds = 30,
            IdleTimeoutSeconds = 60,
            MaxServersPerMachine = 4
        });
        var serverId = Guid.NewGuid();
        db.GameServerInstances.Add(NewServer(
            serverId,
            "IN_PROGRESS",
            7778,
            startedAt: DateTimeOffset.UtcNow.AddMinutes(-10),
            lastHeartbeatAt: DateTimeOffset.UtcNow.AddMinutes(-5)));
        db.PortAllocations.Add(new PortAllocation
        {
            Port = 7778,
            Status = "ALLOCATED",
            ServerId = serverId,
            AllocatedAt = DateTimeOffset.UtcNow.AddMinutes(-10)
        });
        await db.SaveChangesAsync();

        var changed = await service.RunMaintenanceAsync();

        var stored = await db.GameServerInstances.SingleAsync(x => x.Id == serverId);
        var port = await db.PortAllocations.SingleAsync(x => x.Port == 7778);
        Assert.Equal(1, changed);
        Assert.Equal("STOPPED", stored.Status);
        Assert.Equal("FREE", port.Status);
    }

    private static GameDbContext CreateDbContext()
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"server-manager-{Guid.NewGuid()}")
            .ConfigureWarnings(x => x.Ignore(InMemoryEventId.TransactionIgnoredWarning))
            .Options;
        return new GameDbContext(options);
    }

    private static DedicatedServerOrchestrator CreateService(GameDbContext db, DedicatedServerOrchestrationOptions? options = null)
    {
        options ??= new DedicatedServerOrchestrationOptions
        {
            ServerMode = "LocalProcess",
            PublicIp = "127.0.0.1",
            PortRangeStart = 7777,
            PortRangeEnd = 7778,
            UeServerExecutablePath = string.Empty,
            BackendUrl = "http://localhost:8080",
            StartupTimeoutSeconds = 30,
            HeartbeatTimeoutSeconds = 30,
            IdleTimeoutSeconds = 60,
            MaxServersPerMachine = 4
        };

        return new DedicatedServerOrchestrator(db, Options.Create(options), NullLogger<DedicatedServerOrchestrator>.Instance);
    }

    private static GameServerInstance NewServer(
        Guid serverId,
        string status,
        int port,
        DateTimeOffset startedAt,
        DateTimeOffset? lastHeartbeatAt = null)
    {
        return new GameServerInstance
        {
            Id = serverId,
            SessionId = Guid.NewGuid(),
            Mode = "classic",
            MapId = "arena_01",
            Region = "cn",
            Ip = "127.0.0.1",
            Port = port,
            Status = status,
            StartedAt = startedAt,
            LastHeartbeatAt = lastHeartbeatAt,
            CreatedAt = startedAt,
            UpdatedAt = startedAt
        };
    }
}

