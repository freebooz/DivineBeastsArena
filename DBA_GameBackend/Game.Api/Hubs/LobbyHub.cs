/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Microsoft.AspNetCore.SignalR;

namespace Games.Hubs;

public sealed class LobbyHub : Hub
{
    public async Task JoinRoom(Guid roomId, Guid playerId)
    {
        await Groups.AddToGroupAsync(Context.ConnectionId, roomId.ToString());
        await Clients.OthersInGroup(roomId.ToString()).SendAsync("PlayerJoined", playerId);
    }

    public async Task LeaveRoom(Guid roomId, Guid playerId)
    {
        await Groups.RemoveFromGroupAsync(Context.ConnectionId, roomId.ToString());
        await Clients.OthersInGroup(roomId.ToString()).SendAsync("PlayerLeft", playerId);
    }

    public async Task SetReady(Guid roomId, Guid playerId, bool isReady)
    {
        await Clients.OthersInGroup(roomId.ToString()).SendAsync("PlayerReadyChanged", playerId, isReady);
    }

    public async Task NotifyMatchFound(Guid ticketId, Guid sessionId)
    {
        await Clients.Group(ticketId.ToString()).SendAsync("MatchFound", sessionId);
    }

    public async Task NotifySessionCreated(Guid sessionId)
    {
        await Clients.Caller.SendAsync("SessionCreated", sessionId);
    }
}