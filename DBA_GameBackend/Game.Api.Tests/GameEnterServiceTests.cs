/*
中文阅读说明：
- 本文件验证 /api/v1/game/enter 编排层最关键的安全边界：角色必须属于当前账号与当前区服，且 DS 未就绪只能返回 PENDING。
- 测试不触发真实 Dedicated Server、Redis 或网络；这些依赖由服务端集成环境的人工审核负责。
*/

using Game.Api.Services.Session;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Contracts.Session;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging.Abstractions;

namespace Game.Api.Tests;

public sealed class GameEnterServiceTests
{
    [Fact]
    public async Task EnterAsync_WhenCharacterServerDoesNotMatch_RejectsBeforeAllocation()
    {
        await using var db = CreateDatabase();
        var accountId = Guid.NewGuid();
        var characterId = Guid.NewGuid();
        var actualServerId = Guid.NewGuid();
        db.PlayerCharacters.Add(new PlayerCharacter
        {
            Id = characterId,
            PlayerId = accountId,
            ServerId = actualServerId,
            CharacterName = "区服校验角色",
            NormalizedName = "区服校验角色",
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            IsSelected = true
        });
        await db.SaveChangesAsync();

        var allocation = new StubVillageAllocationService();
        var service = new GameEnterService(db, allocation, new StubSessionService(), NullLogger<GameEnterService>.Instance);

        var result = await service.EnterAsync(accountId, new GameEnterRequest(characterId, Guid.NewGuid()));

        Assert.Equal(GameEnterDisposition.Rejected, result.Disposition);
        Assert.False(allocation.WasCalled);
    }

    [Fact]
    public async Task EnterAsync_WhenDedicatedServerIsStillStarting_ReturnsPendingWithoutTicket()
    {
        await using var db = CreateDatabase();
        var accountId = Guid.NewGuid();
        var characterId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        db.PlayerCharacters.Add(new PlayerCharacter
        {
            Id = characterId,
            PlayerId = accountId,
            ServerId = serverId,
            CharacterName = "等待服务器角色",
            NormalizedName = "等待服务器角色",
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            IsSelected = true
        });
        await db.SaveChangesAsync();

        var allocatedSessionId = Guid.NewGuid();
        var allocation = new StubVillageAllocationService(new VillageAllocationResponse(allocatedSessionId, "ALLOCATING_SERVER"));
        var service = new GameEnterService(db, allocation, new StubSessionService(), NullLogger<GameEnterService>.Instance);

        var result = await service.EnterAsync(accountId, new GameEnterRequest(characterId, serverId));

        Assert.Equal(GameEnterDisposition.Pending, result.Disposition);
        Assert.NotNull(result.Response);
        Assert.Equal("PENDING", result.Response!.Status);
        Assert.Equal(allocatedSessionId, result.Response.SessionId);
        Assert.Null(result.Response.Connection);
    }

    [Fact]
    public async Task EnterAsync_WhenCharacterHasNotBeenSelected_RejectsBeforeAllocation()
    {
        // 进入游戏不是“只要知道角色 ID 就能进入”的接口。此测试固定前台必须先完成
        // CharacterSelect 的选择动作，防止旧页面或过期异步回调绕过当前角色选择状态。
        await using var db = CreateDatabase();
        var accountId = Guid.NewGuid();
        var characterId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        db.PlayerCharacters.Add(new PlayerCharacter
        {
            Id = characterId,
            PlayerId = accountId,
            ServerId = serverId,
            CharacterName = "未选择角色",
            NormalizedName = "未选择角色",
            Zodiac = "Tiger",
            PrimaryElement = "Wood",
            FiveCamp = "East",
            IsSelected = false
        });
        await db.SaveChangesAsync();

        var allocation = new StubVillageAllocationService();
        var service = new GameEnterService(db, allocation, new StubSessionService(), NullLogger<GameEnterService>.Instance);

        var result = await service.EnterAsync(accountId, new GameEnterRequest(characterId, serverId));

        Assert.Equal(GameEnterDisposition.Rejected, result.Disposition);
        Assert.False(allocation.WasCalled);
    }

    private static GameDbContext CreateDatabase() => new(new DbContextOptionsBuilder<GameDbContext>()
        .UseInMemoryDatabase($"game-enter-{Guid.NewGuid()}").Options);

    /// <summary>仅记录是否被调用，避免在单元测试中启动真实 DS 编排。</summary>
    private sealed class StubVillageAllocationService(VillageAllocationResponse? response = null) : IVillageAllocationService
    {
        public bool WasCalled { get; private set; }

        public Task<VillageAllocationResponse?> AllocateAsync(Guid playerId, Guid characterId, CancellationToken cancellationToken = default)
        {
            WasCalled = true;
            return Task.FromResult(response);
        }
    }

    /// <summary>仅提供 GameEnterService 使用的连接查询；其余接口保持显式空实现以维持现有 ISessionService 契约。</summary>
    private sealed class StubSessionService : ISessionService
    {
        public Task<SessionResponse?> GetSessionAsync(Guid sessionId) => Task.FromResult<SessionResponse?>(null);
        public Task<SessionConnectionResponse?> GetConnectionInfoAsync(Guid sessionId, Guid playerId) => Task.FromResult<SessionConnectionResponse?>(null);
        public Task<SessionResponse?> CreateFromRoomAsync(Guid roomId) => Task.FromResult<SessionResponse?>(null);
        public Task<SessionResponse?> CreateFromMatchAsync(Guid ticketId) => Task.FromResult<SessionResponse?>(null);
        public Task<SessionResponse?> AllocateServerAsync(Guid sessionId, string ip, int port, string runtimeToken) => Task.FromResult<SessionResponse?>(null);
        public Task<SessionResponse?> MarkInProgressAsync(Guid sessionId) => Task.FromResult<SessionResponse?>(null);
        public Task<SessionResponse?> MarkCompletedAsync(Guid sessionId) => Task.FromResult<SessionResponse?>(null);
        public Task<SessionResponse?> MarkFailedAsync(Guid sessionId, string reason) => Task.FromResult<SessionResponse?>(null);
    }
}
