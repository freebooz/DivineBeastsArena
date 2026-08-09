/*
中文阅读说明：
- 本服务是前台“进入游戏”意图的唯一 API 编排入口，不实现战斗、匹配或 Dedicated Server 运行时规则。
- 它复用 VillageAllocationService 和 SessionService，避免为角色选择页再建立一套会话、服务器分配和票据签发逻辑。
- CharacterId 与 ServerId 均在服务端按当前 AccessToken 的 player_id 复核；客户端不能指定 DS 实例或票据内容。
*/

using Game.Infrastructure.Database;
using Game.Shared.Contracts.Session;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Services.Session;

public enum GameEnterDisposition
{
    Ready,
    Pending,
    Rejected
}

public sealed record GameEnterServiceResult(
    GameEnterDisposition Disposition,
    GameEnterResponse? Response = null,
    string? ErrorMessage = null);

public interface IGameEnterService
{
    Task<GameEnterServiceResult> EnterAsync(
        Guid accountId,
        GameEnterRequest request,
        CancellationToken cancellationToken = default);
}

/// <summary>
/// 把“角色已选中”这一前台业务意图收敛为：归属复核、区服复核、复用既有开发 DS 分配、签发同一张 JoinTicket。
/// </summary>
public sealed class GameEnterService(
    GameDbContext db,
    IVillageAllocationService villageAllocation,
    ISessionService sessionService,
    ILogger<GameEnterService> logger) : IGameEnterService
{
    public async Task<GameEnterServiceResult> EnterAsync(
        Guid accountId,
        GameEnterRequest request,
        CancellationToken cancellationToken = default)
    {
        if (accountId == Guid.Empty || request.CharacterId == Guid.Empty || request.ServerId == Guid.Empty)
        {
            return new(GameEnterDisposition.Rejected, ErrorMessage: "角色或区服标识无效。");
        }

        // 先以账号 + 角色 + 区服三元组过滤，避免泄漏其他账号角色的存在性。
        var character = await db.PlayerCharacters.AsNoTracking()
            .SingleOrDefaultAsync(x => x.Id == request.CharacterId
                && x.PlayerId == accountId
                && x.ServerId == request.ServerId
                && !x.IsDeleted,
                cancellationToken);
        if (character is null)
        {
            logger.LogWarning("进入游戏被拒绝：角色不属于当前账号或区服不匹配。账号={AccountId}，角色={CharacterId}，区服={ServerId}",
                accountId, request.CharacterId, request.ServerId);
            return new(GameEnterDisposition.Rejected, ErrorMessage: "角色不存在、不可用或不属于当前区服。");
        }

        // “已选择”是角色进入当前世界的显式状态，不允许客户端只凭 CharacterId 跳过角色选择流程。
        if (!character.IsSelected)
        {
            return new(GameEnterDisposition.Rejected, ErrorMessage: "请先在角色选择页确认要进入游戏的角色。");
        }

        // VillageAllocationService 保持既有“已选择角色”约束，并负责选择或编排当前开发 Dedicated Server 实例。
        var allocation = await villageAllocation.AllocateAsync(accountId, request.CharacterId, cancellationToken);
        if (allocation is null)
        {
            return new(GameEnterDisposition.Rejected, ErrorMessage: "当前角色尚未处于可进入游戏的选择状态，或服务器资源不可用。");
        }

        // Dedicated Server 可能仍在启动。此时返回可重试的 PENDING，而不是阻塞 HTTP 请求或伪造连接票据。
        var connection = await sessionService.GetConnectionInfoAsync(allocation.SessionId, accountId);
        if (connection is null)
        {
            return new(
                GameEnterDisposition.Pending,
                new GameEnterResponse("PENDING", allocation.SessionId));
        }

        return new(
            GameEnterDisposition.Ready,
            new GameEnterResponse(
                "READY",
                connection.SessionId,
                connection));
    }
}
