/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：测试房间与会话创建的上线关键契约，重点覆盖房主转移和会话幂等创建。
- 阅读重点：测试名称即业务规则；Arrange 准备房间玩家，Act 调用服务，Assert 检查数据库最终状态。
- 修改提示：调整房间状态机或会话来源规则时，请同步更新这些测试。
*/

using Game.Api.Services.Room;
using Game.Api.Services.Session;
using Game.Infrastructure.Database;
using Game.Shared.Contracts.Room;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging.Abstractions;

namespace Game.Api.Tests;

public class RoomSessionServiceTests
{
    [Fact]
    public async Task LeaveRoom_WhenOwnerLeaves_TransfersOwnerToRemainingPlayer()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var ownerId = Guid.NewGuid();
        var nextOwnerId = Guid.NewGuid();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, nextOwnerId, null);

        var left = await roomService.LeaveRoomAsync(room.Id, ownerId);
        var storedRoom = await db.GameRooms.AsNoTracking().SingleAsync(x => x.Id == room.Id);

        Assert.True(left);
        Assert.Equal(nextOwnerId, storedRoom.OwnerPlayerId);
        Assert.Equal("WAITING", storedRoom.Status);
    }

    [Fact]
    public async Task CreateFromRoomAsync_CalledTwice_ReturnsSameSessionAndPlayers()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var sessionService = new SessionService(db, NullLogger<SessionService>.Instance);
        var ownerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, secondPlayerId, null);
        await roomService.SetReadyAsync(room.Id, secondPlayerId, true);
        await roomService.StartGameAsync(room.Id, ownerId);

        var firstSession = await sessionService.CreateFromRoomAsync(room.Id);
        var secondSession = await sessionService.CreateFromRoomAsync(room.Id);

        Assert.NotNull(firstSession);
        Assert.NotNull(secondSession);
        Assert.Equal(firstSession!.Id, secondSession!.Id);
        Assert.Equal(1, await db.GameSessions.CountAsync(x => x.SourceType == "ROOM" && x.SourceId == room.Id));
        Assert.Equal(2, await db.PlayerSessions.CountAsync(x => x.GameSessionId == firstSession.Id));
    }

    private static GameDbContext CreateDbContext()
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"room-session-{Guid.NewGuid()}")
            .Options;
        return new GameDbContext(options);
    }

    private static CreateRoomRequest CreateRoom() => new(
        "classic",
        "arena_01",
        "cn",
        2,
        "PUBLIC",
        null);
}
