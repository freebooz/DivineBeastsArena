/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Room;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Services.Room;

public interface IRoomService
{
    Task<RoomResponse> CreateRoomAsync(CreateRoomRequest request, Guid playerId);
    Task<IReadOnlyList<RoomResponse>> GetRoomsAsync(string mode, string region, string? visibility = null);
    Task<RoomResponse?> GetRoomAsync(Guid roomId);
    Task<RoomResponse?> JoinRoomAsync(Guid roomId, Guid playerId, string? password);
    Task<bool> LeaveRoomAsync(Guid roomId, Guid playerId);
    Task<bool> SetReadyAsync(Guid roomId, Guid playerId, bool isReady);
    Task<RoomResponse?> StartGameAsync(Guid roomId, Guid playerId);
    Task<bool> KickPlayerAsync(Guid roomId, Guid playerId, Guid targetPlayerId);
    Task<RoomResponse?> TransferOwnerAsync(Guid roomId, Guid playerId, Guid newOwnerId);
}

public sealed class RoomService : IRoomService
{
    private readonly GameDbContext _db;
    private readonly ILogger<RoomService> _logger;

    public RoomService(GameDbContext db, ILogger<RoomService> logger)
    {
        _db = db;
        _logger = logger;
    }

    public async Task<RoomResponse> CreateRoomAsync(CreateRoomRequest request, Guid playerId)
    {
        var existingRoom = await _db.GameRoomPlayers
            .Include(x => x.Room)
            .FirstOrDefaultAsync(x => x.PlayerId == playerId && x.LeftAt == null);

        if (existingRoom != null && existingRoom.Room?.Status != "CLOSED")
            throw new InvalidOperationException("Player already in a room");

        var room = new GameRoom
        {
            Id = Guid.NewGuid(),
            OwnerPlayerId = playerId,
            Mode = request.Mode,
            MapId = request.MapId,
            Region = request.Region,
            MaxPlayers = request.MaxPlayers,
            Visibility = request.Visibility,
            Status = "WAITING",
            CreatedAt = DateTimeOffset.UtcNow
        };

        var player = new GameRoomPlayer
        {
            Id = Guid.NewGuid(),
            RoomId = room.Id,
            PlayerId = playerId,
            SlotIndex = 0,
            IsReady = false,
            JoinedAt = DateTimeOffset.UtcNow
        };

        _db.GameRooms.Add(room);
        _db.GameRoomPlayers.Add(player);
        await _db.SaveChangesAsync();

        return await GetRoomAsync(room.Id) ?? throw new Exception("Failed to create room");
    }

    public async Task<IReadOnlyList<RoomResponse>> GetRoomsAsync(string mode, string region, string? visibility = null)
    {
        var query = _db.GameRooms
            .Include(x => x.Players.Where(p => p.LeftAt == null))
            .Where(x => x.Mode == mode && x.Region == region && x.Status == "WAITING");

        if (!string.IsNullOrEmpty(visibility))
            query = query.Where(x => x.Visibility == visibility);

        var rooms = await query.OrderByDescending(x => x.CreatedAt).Take(50).ToListAsync();
        return rooms.Select(ToResponse).ToList();
    }

    public async Task<RoomResponse?> GetRoomAsync(Guid roomId)
    {
        var room = await _db.GameRooms
            .Include(x => x.Players.Where(p => p.LeftAt == null))
            .FirstOrDefaultAsync(x => x.Id == roomId);

        return room == null ? null : ToResponse(room);
    }

    public async Task<RoomResponse?> JoinRoomAsync(Guid roomId, Guid playerId, string? password)
    {
        var room = await _db.GameRooms
            .Include(x => x.Players.Where(p => p.LeftAt == null))
            .FirstOrDefaultAsync(x => x.Id == roomId);

        if (room == null) return null;
        if (room.Status != "WAITING") throw new InvalidOperationException(ErrorCodes.RoomInProgress);
        if (room.Players.Count >= room.MaxPlayers) throw new InvalidOperationException(ErrorCodes.RoomFull);

        if (room.Players.Any(x => x.PlayerId == playerId))
            throw new InvalidOperationException(ErrorCodes.RoomAlreadyJoined);

        var slotIndex = Enumerable.Range(0, room.MaxPlayers)
            .First(i => !room.Players.Any(p => p.SlotIndex == i));

        var player = new GameRoomPlayer
        {
            Id = Guid.NewGuid(),
            RoomId = roomId,
            PlayerId = playerId,
            SlotIndex = slotIndex,
            IsReady = false,
            JoinedAt = DateTimeOffset.UtcNow
        };

        _db.GameRoomPlayers.Add(player);
        await _db.SaveChangesAsync();

        return await GetRoomAsync(roomId);
    }

    public async Task<bool> LeaveRoomAsync(Guid roomId, Guid playerId)
    {
        var player = await _db.GameRoomPlayers
            .FirstOrDefaultAsync(x => x.RoomId == roomId && x.PlayerId == playerId && x.LeftAt == null);

        if (player == null) return false;

        player.LeftAt = DateTimeOffset.UtcNow;

        var room = await _db.GameRooms.FindAsync(roomId);
        if (room != null)
        {
            if (room.OwnerPlayerId == playerId && room.Players.Count(p => p.LeftAt == null) > 1)
            {
                var newOwner = room.Players.FirstOrDefault(p => p.LeftAt == null && p.PlayerId != playerId);
                if (newOwner != null)
                    room.OwnerPlayerId = newOwner.PlayerId;
            }

            if (!room.Players.Any(p => p.LeftAt == null))
                room.Status = "CLOSED";
        }

        await _db.SaveChangesAsync();
        return true;
    }

    public async Task<bool> SetReadyAsync(Guid roomId, Guid playerId, bool isReady)
    {
        var player = await _db.GameRoomPlayers
            .FirstOrDefaultAsync(x => x.RoomId == roomId && x.PlayerId == playerId && x.LeftAt == null);

        if (player == null) return false;

        player.IsReady = isReady;
        await _db.SaveChangesAsync();
        return true;
    }

    public async Task<RoomResponse?> StartGameAsync(Guid roomId, Guid playerId)
    {
        var room = await _db.GameRooms
            .Include(x => x.Players.Where(p => p.LeftAt == null))
            .FirstOrDefaultAsync(x => x.Id == roomId);

        if (room == null) return null;
        if (room.OwnerPlayerId != playerId) throw new UnauthorizedAccessException(ErrorCodes.RoomNotOwner);

        var allReady = room.Players.All(p => p.IsReady || p.PlayerId == playerId);
        if (!allReady) throw new InvalidOperationException(ErrorCodes.RoomNotReady);

        var minPlayers = room.Mode.Contains("5v5") ? 10 : 2;
        if (room.Players.Count < minPlayers)
            throw new InvalidOperationException($"Need at least {minPlayers} players to start");

        room.Status = "STARTING";
        room.UpdatedAt = DateTimeOffset.UtcNow;
        await _db.SaveChangesAsync();

        return ToResponse(room);
    }

    public async Task<bool> KickPlayerAsync(Guid roomId, Guid playerId, Guid targetPlayerId)
    {
        var room = await _db.GameRooms.FindAsync(roomId);
        if (room == null || room.OwnerPlayerId != playerId) return false;

        var target = await _db.GameRoomPlayers
            .FirstOrDefaultAsync(x => x.RoomId == roomId && x.PlayerId == targetPlayerId && x.LeftAt == null);

        if (target == null) return false;

        target.LeftAt = DateTimeOffset.UtcNow;
        await _db.SaveChangesAsync();
        return true;
    }

    public async Task<RoomResponse?> TransferOwnerAsync(Guid roomId, Guid playerId, Guid newOwnerId)
    {
        var room = await _db.GameRooms.FindAsync(roomId);
        if (room == null || room.OwnerPlayerId != playerId) return null;

        var newOwner = room.Players.FirstOrDefault(p => p.LeftAt == null && p.PlayerId == newOwnerId);
        if (newOwner == null) return null;

        room.OwnerPlayerId = newOwnerId;
        room.UpdatedAt = DateTimeOffset.UtcNow;
        await _db.SaveChangesAsync();

        return ToResponse(room);
    }

    private static RoomResponse ToResponse(GameRoom room)
    {
        var players = room.Players?.Select(p =>
            new RoomPlayerDto(p.PlayerId, "", p.SlotIndex, p.Team, p.IsReady)).ToList()
            ?? new List<RoomPlayerDto>();

        return new RoomResponse(room.Id, room.OwnerPlayerId, room.Mode, room.MapId,
            room.Region, room.MaxPlayers, room.Visibility, room.Status, players, room.CreatedAt);
    }
}