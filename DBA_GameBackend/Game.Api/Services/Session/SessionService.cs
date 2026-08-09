/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：作为会话 HTTP 服务适配器，调用 Application 用例并映射共享响应 DTO。
- 架构约束：不得直接访问 EF Core、数据库实体或实现会话状态机；业务编排位于 Game.Application。
*/

using Game.Application.Sessions;
using Game.Infrastructure.Redis;
using Game.Shared.Contracts.Session;

namespace Game.Api.Services.Session;

public interface ISessionService
{
    Task<SessionResponse?> GetSessionAsync(Guid sessionId);
    Task<SessionConnectionResponse?> GetConnectionInfoAsync(Guid sessionId, Guid playerId);
    Task<SessionResponse?> CreateFromRoomAsync(Guid roomId);
    Task<SessionResponse?> CreateFromMatchAsync(Guid ticketId);
    Task<SessionResponse?> AllocateServerAsync(Guid sessionId, string ip, int port, string runtimeToken);
    Task<SessionResponse?> MarkInProgressAsync(Guid sessionId);
    Task<SessionResponse?> MarkCompletedAsync(Guid sessionId);
    Task<SessionResponse?> MarkFailedAsync(Guid sessionId, string reason);
}
public sealed class SessionService : ISessionService
{
    private readonly ILogger<SessionService> _logger;
    private readonly IGetSessionUseCase _getSession;
    private readonly ICreateSessionFromRoomUseCase _createSessionFromRoom;
    private readonly ICreateSessionFromMatchUseCase _createSessionFromMatch;
    private readonly IIssueSessionConnectionUseCase _issueSessionConnection;
    private readonly IChangeSessionLifecycleUseCase _changeSessionLifecycle;
    private readonly IAllocateSessionServerUseCase _allocateSessionServer;
    private readonly IGameTicketRedisRegistry? _gameTicketRegistry;

    public SessionService(
        ILogger<SessionService> logger,
        IGetSessionUseCase getSession,
        ICreateSessionFromRoomUseCase createSessionFromRoom,
        ICreateSessionFromMatchUseCase createSessionFromMatch,
        IIssueSessionConnectionUseCase issueSessionConnection,
        IChangeSessionLifecycleUseCase changeSessionLifecycle,
        IAllocateSessionServerUseCase allocateSessionServer,
        IGameTicketRedisRegistry? gameTicketRegistry = null)
    {
        _logger = logger;
        _getSession = getSession;
        _createSessionFromRoom = createSessionFromRoom;
        _createSessionFromMatch = createSessionFromMatch;
        _issueSessionConnection = issueSessionConnection;
        _changeSessionLifecycle = changeSessionLifecycle;
        _allocateSessionServer = allocateSessionServer;
        _gameTicketRegistry = gameTicketRegistry;
    }

    public async Task<SessionResponse?> GetSessionAsync(Guid sessionId)
    {
        var state = await _getSession.ExecuteAsync(sessionId);
        return state is null ? null : ToResponse(state);
    }

    public async Task<SessionConnectionResponse?> GetConnectionInfoAsync(Guid sessionId, Guid playerId)
    {
        var connection = await _issueSessionConnection.ExecuteAsync(sessionId, playerId);
        if (connection is null)
        {
            _logger.LogWarning("会话连接信息签发失败。会话={SessionId} 玩家={PlayerId}", sessionId, playerId);
            return null;
        }

        // 生产 DI 必定注入 Redis 索引。可空参数仅保留给现有纯内存用例测试，避免其伪造 Redis 网络依赖。
        // Redis 写入失败时不把票据交给客户端，防止 Dedicated Server 无法执行原子消费校验。
        if (_gameTicketRegistry is not null)
        {
            try
            {
                if (!await _gameTicketRegistry.RecordIssuedAsync(connection))
                {
                    _logger.LogWarning("入服票据 Redis 绑定写入失败，会话={SessionId}。", sessionId);
                    return null;
                }
            }
            catch (Exception exception)
            {
                _logger.LogWarning(exception, "入服票据 Redis 绑定异常，会话={SessionId}。", sessionId);
                return null;
            }
        }

        return new SessionConnectionResponse(
            connection.SessionId,
            connection.ServerIp,
            connection.ServerPort,
            connection.JoinTicket,
            connection.JoinTicketExpiresAt,
            connection.PlayerId,
            connection.CharacterId,
            connection.ServerInstanceId,
            connection.BuildId,
            connection.TeamId,
            connection.Build,
            connection.ReconnectToken,
            connection.ReconnectTokenExpiresAt);
    }

    public async Task<SessionResponse?> CreateFromRoomAsync(Guid roomId)
    {
        var state = await _createSessionFromRoom.ExecuteAsync(roomId);
        return state is null ? null : ToResponse(state);
    }

    public async Task<SessionResponse?> CreateFromMatchAsync(Guid ticketId)
    {
        var state = await _createSessionFromMatch.ExecuteAsync(ticketId);
        return state is null ? null : ToResponse(state);
    }

    public async Task<SessionResponse?> AllocateServerAsync(Guid sessionId, string ip, int port, string runtimeToken)
    {
        var state = await _allocateSessionServer.ExecuteAsync(
            sessionId,
            ip,
            port,
            runtimeToken);
        if (state is null)
        {
            _logger.LogWarning("服务器地址分配失败：会话状态、服务器绑定或运行时令牌无效。会话={SessionId}", sessionId);
            return null;
        }
        return ToResponse(state);
    }

    public async Task<SessionResponse?> MarkInProgressAsync(Guid sessionId)
    {
        var state = await _changeSessionLifecycle.ExecuteAsync(
            sessionId,
            SessionLifecycleTransition.Start);
        return state is null ? null : ToResponse(state);
    }

    public async Task<SessionResponse?> MarkCompletedAsync(Guid sessionId)
    {
        var state = await _changeSessionLifecycle.ExecuteAsync(
            sessionId,
            SessionLifecycleTransition.Complete);
        return state is null ? null : ToResponse(state);
    }

    public async Task<SessionResponse?> MarkFailedAsync(Guid sessionId, string reason)
    {
        var state = await _changeSessionLifecycle.ExecuteAsync(
            sessionId,
            SessionLifecycleTransition.Fail,
            reason);
        return state is null ? null : ToResponse(state);
    }

    private static SessionResponse ToResponse(SessionLifecycleState s) => new(
        s.SessionId, s.SourceType, s.Mode, s.MapId, s.Region, s.Status,
        s.ServerIp, s.ServerPort, s.MaxPlayers, s.CreatedAt, s.StartedAt);

}
