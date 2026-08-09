/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Player;

namespace Game.Api.Services.Player;

public interface IPlayerService
{
    /**
     * 确保玩家拥有服务端生成的游戏名。该接口只处理 PlayerProfile.Nickname，绝不修改
     * 账号登录名或 PlayerIdentity.DisplayName；重复调用幂等，适用于首次登录和显式补偿接口。
     */
    Task<PlayerGameNameEnsureResult> EnsureGeneratedGameNameAsync(Guid playerId, CancellationToken cancellationToken = default);
    Task<PlayerProfileResponse?> GetProfileAsync(Guid playerId);
    Task<PlayerProfileResponse?> UpdateProfileAsync(Guid playerId, UpdateProfileRequest request);
    Task<PlayerSettingsResponse?> GetSettingsAsync(Guid playerId);
    Task<PlayerSettingsResponse?> UpdateSettingsAsync(Guid playerId, UpdateSettingsRequest request);
    Task<PlayerStatisticsResponse?> GetStatisticsAsync(Guid playerId);
    Task<PlayerPublicProfileResponse?> GetPublicProfileAsync(Guid playerId);
    Task<PlayerUnlocksResponse?> GetUnlocksAsync(Guid playerId);
    Task<bool> IsNicknameAvailableAsync(string nickname, Guid? excludePlayerId = null);
    Task<bool> CanUpdateNicknameAsync(Guid playerId);
}

/** 自动生成游戏玩家名的结果，不携带认证凭据或账号敏感信息。 */
public sealed record PlayerGameNameEnsureResult(bool Success, string? Nickname = null, bool WasGenerated = false, string? ErrorMessage = null);

public sealed class NicknameValidationResult(bool IsValid, string? ErrorCode = null, string? ErrorMessage = null)
{
    public bool IsValid { get; } = IsValid;
    public string? ErrorCode { get; } = ErrorCode;
    public string? ErrorMessage { get; } = ErrorMessage;
}

public static class NicknameValidator
{
    public static NicknameValidationResult Validate(string nickname)
    {
        if (string.IsNullOrWhiteSpace(nickname))
            return new(false, "PLAYER_INVALID_NICKNAME", "Nickname is required");

        if (nickname.Length < 2 || nickname.Length > 16)
            return new(false, "PLAYER_INVALID_NICKNAME", "Nickname must be 2-16 characters");

        foreach (var c in nickname)
        {
            if (!IsValidChar(c))
                return new(false, "PLAYER_INVALID_NICKNAME", "Nickname can only contain Chinese, English characters, numbers and underscore");
        }

        return new(true);
    }

    private static bool IsValidChar(char c) =>
        char.IsLetterOrDigit(c) || (c >= 0x4E00 && c <= 0x9FFF);
}
