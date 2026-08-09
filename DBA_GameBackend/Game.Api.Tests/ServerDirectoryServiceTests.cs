using Game.Api.Services.ServerDirectory;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Contracts.GameServer;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging.Abstractions;
using Microsoft.Extensions.Options;

namespace Game.Api.Tests;

public sealed class ServerDirectoryServiceTests
{
    [Fact]
    public async Task GetServers_ReturnsStableIdsAndPlacesSelectableRecommendedServersFirst()
    {
        await using var db = CreateContext();
        await db.GameServers.AddRangeAsync(
            NewServer("10000000-0000-0000-0000-000000000001", "维护区", ServerDirectoryStatuses.Maintenance, 0, true),
            NewServer("10000000-0000-0000-0000-000000000002", "普通一区", ServerDirectoryStatuses.Online, 30, false),
            NewServer("10000000-0000-0000-0000-000000000003", "推荐一区", ServerDirectoryStatuses.Busy, 150, true));
        await db.SaveChangesAsync();

        var servers = await CreateService(db).GetServersAsync(new ServerDirectoryQuery("cn-east", null, "windows"), CancellationToken.None);

        Assert.Equal("10000000-0000-0000-0000-000000000003", servers[0].ServerId.ToString());
        Assert.True(servers[0].CanSelect);
        Assert.Equal("10000000-0000-0000-0000-000000000002", servers[1].ServerId.ToString());
        Assert.False(servers[2].CanSelect);
        Assert.Equal("维护区", servers[2].Name);
    }

    [Fact]
    public async Task GetServers_MaintenanceAndOfflineAreExposedButCannotBeSelectedAndVersionFilterApplies()
    {
        await using var db = CreateContext();
        await db.GameServers.AddRangeAsync(
            NewServer("20000000-0000-0000-0000-000000000001", "维护区", ServerDirectoryStatuses.Maintenance, 0, false, maintenanceMessage: "维护中"),
            NewServer("20000000-0000-0000-0000-000000000002", "离线区", ServerDirectoryStatuses.Offline, 0, false),
            NewServer("20000000-0000-0000-0000-000000000003", "预更新区", ServerDirectoryStatuses.Online, 5, true, minClientVersion: "5.8.1"));
        await db.SaveChangesAsync();

        var service = CreateService(db);
        var compatible = await service.GetServersAsync(new ServerDirectoryQuery("cn-east", "5.8.1", "windows"), CancellationToken.None);
        var incompatible = await service.GetServersAsync(new ServerDirectoryQuery("cn-east", "5.8.0", "windows"), CancellationToken.None);

        Assert.Contains(compatible, server => server.Name == "预更新区" && server.CanSelect);
        Assert.Contains(compatible, server => server.Name == "维护区" && !server.CanSelect && server.MaintenanceMessage == "维护中");
        Assert.Contains(compatible, server => server.Name == "离线区" && !server.CanSelect);
        Assert.DoesNotContain(incompatible, server => server.Name == "预更新区");
    }

    private static GameDbContext CreateContext()
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"server-directory-{Guid.NewGuid()}")
            .Options;
        return new GameDbContext(options);
    }

    private static ServerDirectoryService CreateService(GameDbContext db)
    {
        var serviceProvider = new ServiceCollection().BuildServiceProvider();
        return new ServerDirectoryService(
            db,
            serviceProvider,
            Options.Create(new ServerDirectoryOptions { CacheTtlSeconds = 5 }),
            NullLogger<ServerDirectoryService>.Instance);
    }

    private static GameServerDirectoryEntry NewServer(
        string id,
        string name,
        string status,
        int population,
        bool recommended,
        string? maintenanceMessage = null,
        string? minClientVersion = null) => new()
    {
        Id = Guid.Parse(id),
        Name = name,
        Region = "cn-east",
        Platform = "ALL",
        Status = status,
        Population = population,
        Recommended = recommended,
        MaintenanceMessage = maintenanceMessage,
        MinClientVersion = minClientVersion,
        CreatedAt = DateTimeOffset.UtcNow,
        UpdatedAt = DateTimeOffset.UtcNow
    };
}
