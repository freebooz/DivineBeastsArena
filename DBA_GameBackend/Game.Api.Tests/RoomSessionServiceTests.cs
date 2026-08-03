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
using System.Security.Cryptography;
using System.Text;

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
        var sessionService = CreateSessionService(db);
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

    [Fact]
    public async Task GetConnectionInfoAsync_ReissuesShortLivedPlayerSessionToken()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var sessionService = CreateSessionService(db);
        var ownerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, secondPlayerId, null);
        await roomService.SetReadyAsync(room.Id, secondPlayerId, true);
        await roomService.StartGameAsync(room.Id, ownerId);
        var session = await sessionService.CreateFromRoomAsync(room.Id);
        Assert.NotNull(session);

        var storedSession = await db.GameSessions.SingleAsync(x => x.Id == session!.Id);
        storedSession.ServerIp = "127.0.0.1";
        storedSession.ServerPort = 7777;
        storedSession.Status = "WAITING_PLAYERS";
        await db.SaveChangesAsync();

        var connection = await sessionService.GetConnectionInfoAsync(session!.Id, ownerId);
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == session.Id && x.PlayerId == ownerId);

        Assert.NotNull(connection);
        Assert.Equal(ownerId, connection!.PlayerId);
        Assert.Equal("127.0.0.1", connection.ServerIp);
        Assert.Equal(7777, connection.ServerPort);
        Assert.False(string.IsNullOrWhiteSpace(connection.PlayerSessionToken));
        Assert.Equal(HashToken(connection.PlayerSessionToken), playerSession.SessionTokenHash);
        Assert.True(connection.TokenExpiresAt <= DateTimeOffset.UtcNow.AddMinutes(11));

        // 重连令牌应同时被签发并持久化 Hash，明文仅返回给客户端
        Assert.False(string.IsNullOrWhiteSpace(connection.ReconnectToken));
        Assert.NotNull(connection.ReconnectTokenExpiresAt);
        Assert.Equal(HashToken(connection.ReconnectToken!), playerSession.ReconnectTokenHash);
        Assert.NotNull(playerSession.ReconnectTokenExpiresAt);
        Assert.True(playerSession.ReconnectTokenExpiresAt <= DateTimeOffset.UtcNow.AddHours(1).AddMinutes(1));
        Assert.True(playerSession.ReconnectTokenExpiresAt > DateTimeOffset.UtcNow.AddMinutes(11));
    }

    [Fact]
    public async Task GetConnectionInfoAsync_ReissuesReconnectTokenOnEachCall()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var sessionService = CreateSessionService(db);
        var ownerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, secondPlayerId, null);
        await roomService.SetReadyAsync(room.Id, secondPlayerId, true);
        await roomService.StartGameAsync(room.Id, ownerId);
        var session = await sessionService.CreateFromRoomAsync(room.Id);
        Assert.NotNull(session);

        var storedSession = await db.GameSessions.SingleAsync(x => x.Id == session!.Id);
        storedSession.ServerIp = "127.0.0.1";
        storedSession.ServerPort = 7777;
        storedSession.Status = "WAITING_PLAYERS";
        await db.SaveChangesAsync();

        // 第一次获取连接信息，应签发重连令牌
        var firstConnection = await sessionService.GetConnectionInfoAsync(session!.Id, ownerId);
        var firstPlayerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == session.Id && x.PlayerId == ownerId);
        Assert.NotNull(firstConnection);
        Assert.False(string.IsNullOrWhiteSpace(firstConnection!.ReconnectToken));
        var firstReconnectHash = firstPlayerSession.ReconnectTokenHash;

        // 第二次获取连接信息，应重新签发重连令牌（覆盖旧值）
        var secondConnection = await sessionService.GetConnectionInfoAsync(session!.Id, ownerId);
        var secondPlayerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == session.Id && x.PlayerId == ownerId);
        Assert.NotNull(secondConnection);
        Assert.False(string.IsNullOrWhiteSpace(secondConnection!.ReconnectToken));
        Assert.NotEqual(firstConnection.ReconnectToken, secondConnection.ReconnectToken);
        Assert.NotEqual(firstReconnectHash, secondPlayerSession.ReconnectTokenHash);
        Assert.Equal(HashToken(secondConnection.ReconnectToken!), secondPlayerSession.ReconnectTokenHash);
    }

    [Fact]
    public async Task GetConnectionInfoAsync_WhenServerIsAllocatedButNotReady_ReturnsNullWithoutReissuingToken()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var sessionService = CreateSessionService(db);
        var ownerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, secondPlayerId, null);
        await roomService.SetReadyAsync(room.Id, secondPlayerId, true);
        await roomService.StartGameAsync(room.Id, ownerId);
        var session = await sessionService.CreateFromRoomAsync(room.Id);
        Assert.NotNull(session);

        var storedSession = await db.GameSessions.SingleAsync(x => x.Id == session!.Id);
        storedSession.ServerIp = "127.0.0.1";
        storedSession.ServerPort = 7777;
        storedSession.Status = "ALLOCATING_SERVER";
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == session.Id && x.PlayerId == ownerId);
        var originalTokenHash = playerSession.SessionTokenHash;
        var originalTokenExpiresAt = playerSession.SessionTokenExpiresAt;
        await db.SaveChangesAsync();

        var connection = await sessionService.GetConnectionInfoAsync(session!.Id, ownerId);
        playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == session.Id && x.PlayerId == ownerId);

        Assert.Null(connection);
        Assert.Equal(originalTokenHash, playerSession.SessionTokenHash);
        Assert.Equal(originalTokenExpiresAt, playerSession.SessionTokenExpiresAt);
    }

    [Fact]
    public async Task GetConnectionInfoAsync_ReturnsFrozenSelectedCharacterBuildSummary()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var sessionService = CreateSessionService(db);
        var ownerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        db.PlayerCharacters.Add(new()
        {
            Id = Guid.NewGuid(),
            PlayerId = ownerId,
            CharacterName = "FlowArenaRat",
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            FixedSkillGroupId = "Rat_Water",
            IsSelected = true,
            CoreAttributesJson = "{}",
            CreatedAt = DateTimeOffset.UtcNow,
            LastUsedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, secondPlayerId, null);
        await roomService.SetReadyAsync(room.Id, secondPlayerId, true);
        await roomService.StartGameAsync(room.Id, ownerId);
        var session = await sessionService.CreateFromRoomAsync(room.Id);
        Assert.NotNull(session);
        var sessionId = session!.Id;

        var storedSession = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        storedSession.ServerIp = "127.0.0.1";
        storedSession.ServerPort = 7777;
        storedSession.Status = "WAITING_PLAYERS";
        await db.SaveChangesAsync();

        var connection = await sessionService.GetConnectionInfoAsync(session!.Id, ownerId);
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == ownerId);

        Assert.NotNull(connection);
        Assert.NotNull(connection!.CharacterBuildSummary);
        Assert.Equal("Rat", connection.CharacterBuildSummary!.Zodiac);
        Assert.Equal("Water", connection.CharacterBuildSummary.PrimaryElement);
        Assert.Equal("East", connection.CharacterBuildSummary.FiveCamp);
        Assert.Equal("Rat_Water", connection.CharacterBuildSummary.FixedSkillGroupId);
        Assert.Equal(1, connection.TeamId);
        Assert.Equal("Rat_Water", playerSession.FixedSkillGroupId);
    }

    [Fact]
    public async Task GetConnectionInfoAsync_WhenSelectedCharacterBuildSummaryIsPadded_ReturnsNormalizedSummary()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var sessionService = CreateSessionService(db);
        var ownerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        db.PlayerCharacters.Add(new()
        {
            Id = Guid.NewGuid(),
            PlayerId = ownerId,
            CharacterName = "FlowArenaPaddedRat",
            Zodiac = " Rat ",
            PrimaryElement = " Water ",
            FiveCamp = " East ",
            FixedSkillGroupId = " Rat_Water ",
            IsSelected = true,
            CoreAttributesJson = "{}",
            CreatedAt = DateTimeOffset.UtcNow,
            LastUsedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, secondPlayerId, null);
        await roomService.SetReadyAsync(room.Id, secondPlayerId, true);
        await roomService.StartGameAsync(room.Id, ownerId);
        var session = await sessionService.CreateFromRoomAsync(room.Id);
        Assert.NotNull(session);
        var sessionId = session!.Id;

        var storedSession = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        storedSession.ServerIp = "127.0.0.1";
        storedSession.ServerPort = 7777;
        storedSession.Status = "WAITING_PLAYERS";
        await db.SaveChangesAsync();

        var connection = await sessionService.GetConnectionInfoAsync(session!.Id, ownerId);
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == ownerId);

        Assert.NotNull(connection);
        Assert.NotNull(connection!.CharacterBuildSummary);
        Assert.Equal("Rat", connection.CharacterBuildSummary!.Zodiac);
        Assert.Equal("Water", connection.CharacterBuildSummary.PrimaryElement);
        Assert.Equal("East", connection.CharacterBuildSummary.FiveCamp);
        Assert.Equal("Rat_Water", connection.CharacterBuildSummary.FixedSkillGroupId);
        Assert.Equal("Rat", playerSession.Zodiac);
        Assert.Equal("Water", playerSession.PrimaryElement);
        Assert.Equal("East", playerSession.FiveCamp);
        Assert.Equal("Rat_Water", playerSession.FixedSkillGroupId);
    }

    [Fact]
    public async Task GetConnectionInfoAsync_WhenExistingFrozenBuildSummaryIsPadded_PersistsNormalizedSummary()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var sessionService = CreateSessionService(db);
        var ownerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, secondPlayerId, null);
        await roomService.SetReadyAsync(room.Id, secondPlayerId, true);
        await roomService.StartGameAsync(room.Id, ownerId);
        var session = await sessionService.CreateFromRoomAsync(room.Id);
        Assert.NotNull(session);
        var sessionId = session!.Id;

        var storedSession = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        storedSession.ServerIp = "127.0.0.1";
        storedSession.ServerPort = 7777;
        storedSession.Status = "WAITING_PLAYERS";

        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == ownerId);
        playerSession.Zodiac = " Rat ";
        playerSession.PrimaryElement = " Water ";
        playerSession.FiveCamp = " East ";
        playerSession.FixedSkillGroupId = " Rat_Water ";
        await db.SaveChangesAsync();

        var connection = await sessionService.GetConnectionInfoAsync(sessionId, ownerId);
        playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == ownerId);

        Assert.NotNull(connection);
        Assert.NotNull(connection!.CharacterBuildSummary);
        Assert.Equal("Rat", connection.CharacterBuildSummary!.Zodiac);
        Assert.Equal("Water", connection.CharacterBuildSummary.PrimaryElement);
        Assert.Equal("East", connection.CharacterBuildSummary.FiveCamp);
        Assert.Equal("Rat_Water", connection.CharacterBuildSummary.FixedSkillGroupId);
        Assert.Equal("Rat", playerSession.Zodiac);
        Assert.Equal("Water", playerSession.PrimaryElement);
        Assert.Equal("East", playerSession.FiveCamp);
        Assert.Equal("Rat_Water", playerSession.FixedSkillGroupId);
    }

    [Fact]
    public async Task GetConnectionInfoAsync_WhenExistingFrozenFixedSkillGroupIsTampered_ReturnsNull()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var sessionService = CreateSessionService(db);
        var ownerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, secondPlayerId, null);
        await roomService.SetReadyAsync(room.Id, secondPlayerId, true);
        await roomService.StartGameAsync(room.Id, ownerId);
        var session = await sessionService.CreateFromRoomAsync(room.Id);
        Assert.NotNull(session);
        var sessionId = session!.Id;

        var storedSession = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        storedSession.ServerIp = "127.0.0.1";
        storedSession.ServerPort = 7777;
        storedSession.Status = "WAITING_PLAYERS";

        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == ownerId);
        playerSession.Zodiac = "Rat";
        playerSession.PrimaryElement = "Water";
        playerSession.FiveCamp = "East";
        playerSession.FixedSkillGroupId = "Tiger_Fire";
        await db.SaveChangesAsync();

        var connection = await sessionService.GetConnectionInfoAsync(sessionId, ownerId);
        playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == ownerId);

        Assert.Null(connection);
        Assert.Equal("Tiger_Fire", playerSession.FixedSkillGroupId);
    }

    [Fact]
    public async Task GetConnectionInfoAsync_WhenExistingFrozenBuildSummaryIsPartial_ReturnsNull()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var sessionService = CreateSessionService(db);
        var ownerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        db.PlayerCharacters.Add(new()
        {
            Id = Guid.NewGuid(),
            PlayerId = ownerId,
            CharacterName = "FlowArenaPartialFrozen",
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            FixedSkillGroupId = "Rat_Water",
            IsSelected = true,
            CoreAttributesJson = "{}",
            CreatedAt = DateTimeOffset.UtcNow,
            LastUsedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, secondPlayerId, null);
        await roomService.SetReadyAsync(room.Id, secondPlayerId, true);
        await roomService.StartGameAsync(room.Id, ownerId);
        var session = await sessionService.CreateFromRoomAsync(room.Id);
        Assert.NotNull(session);
        var sessionId = session!.Id;

        var storedSession = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        storedSession.ServerIp = "127.0.0.1";
        storedSession.ServerPort = 7777;
        storedSession.Status = "WAITING_PLAYERS";

        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == ownerId);
        playerSession.Zodiac = "Rat";
        playerSession.PrimaryElement = null;
        playerSession.FiveCamp = "East";
        playerSession.FixedSkillGroupId = "Rat_Water";
        await db.SaveChangesAsync();

        var connection = await sessionService.GetConnectionInfoAsync(sessionId, ownerId);
        playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == ownerId);

        Assert.Null(connection);
        Assert.Null(playerSession.PrimaryElement);
        Assert.Equal("Rat_Water", playerSession.FixedSkillGroupId);
    }

    [Fact]
    public async Task GetConnectionInfoAsync_WhenSelectedCharacterFixedSkillGroupIsTampered_FreezesComputedSkillGroup()
    {
        await using var db = CreateDbContext();
        var roomService = new RoomService(db, NullLogger<RoomService>.Instance);
        var sessionService = CreateSessionService(db);
        var ownerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        db.PlayerCharacters.Add(new()
        {
            Id = Guid.NewGuid(),
            PlayerId = ownerId,
            CharacterName = "FlowArenaTamperedSource",
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            FixedSkillGroupId = "Tiger_Fire",
            IsSelected = true,
            CoreAttributesJson = "{}",
            CreatedAt = DateTimeOffset.UtcNow,
            LastUsedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();

        var room = await roomService.CreateRoomAsync(CreateRoom(), ownerId);
        await roomService.JoinRoomAsync(room.Id, secondPlayerId, null);
        await roomService.SetReadyAsync(room.Id, secondPlayerId, true);
        await roomService.StartGameAsync(room.Id, ownerId);
        var session = await sessionService.CreateFromRoomAsync(room.Id);
        Assert.NotNull(session);
        var sessionId = session!.Id;

        var storedSession = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        storedSession.ServerIp = "127.0.0.1";
        storedSession.ServerPort = 7777;
        storedSession.Status = "WAITING_PLAYERS";
        await db.SaveChangesAsync();

        var connection = await sessionService.GetConnectionInfoAsync(sessionId, ownerId);
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == ownerId);

        Assert.NotNull(connection);
        Assert.Equal("Rat_Water", connection!.CharacterBuildSummary!.FixedSkillGroupId);
        Assert.Equal("Rat_Water", playerSession.FixedSkillGroupId);
    }

    private static SessionService CreateSessionService(GameDbContext db)
    {
        var characterBuildPolicy = TestCharacterBuildFactory.CreatePolicy();
        var credentialIssuer = TestCharacterBuildFactory.CreateCredentialIssuer();
        var issueSessionConnection = new Game.Application.Sessions.IssueSessionConnectionUseCase(
            new Game.Infrastructure.Database.Admissions.EfSessionAdmissionStore(db),
            characterBuildPolicy,
            credentialIssuer);
        var changeSessionLifecycle = new Game.Application.Sessions.ChangeSessionLifecycleUseCase(
            new Game.Infrastructure.Database.Sessions.EfSessionLifecycleStore(db),
            TimeProvider.System);
        var allocateSessionServer = new Game.Application.Sessions.AllocateSessionServerUseCase(
            new Game.Infrastructure.Database.Sessions.EfSessionServerAllocationStore(db),
            TimeProvider.System);
        var sessionCreationStore = new Game.Infrastructure.Database.Sessions.EfSessionCreationStore(db);
        var createSessionFromRoom = new Game.Application.Sessions.CreateSessionFromRoomUseCase(
            sessionCreationStore,
            characterBuildPolicy,
            credentialIssuer,
            TimeProvider.System);
        var createSessionFromMatch = new Game.Application.Sessions.CreateSessionFromMatchUseCase(
            sessionCreationStore,
            characterBuildPolicy,
            credentialIssuer,
            TestCharacterBuildFactory.SessionAdmissionOptions,
            TimeProvider.System);
        var getSession = new Game.Application.Sessions.GetSessionUseCase(
            new Game.Infrastructure.Database.Sessions.EfSessionQueryStore(db));
        return new SessionService(
            NullLogger<SessionService>.Instance,
            getSession,
            createSessionFromRoom,
            createSessionFromMatch,
            issueSessionConnection,
            changeSessionLifecycle,
            allocateSessionServer);
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

    private static string HashToken(string token)
    {
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(token));
        return Convert.ToHexString(hash).ToLowerInvariant();
    }
}
