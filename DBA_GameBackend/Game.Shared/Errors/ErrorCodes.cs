/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Shared.Errors;

public static class ErrorCodes
{
    public const string AuthInvalidCredentials = "AUTH_INVALID_CREDENTIALS";
    public const string AuthTokenExpired = "AUTH_TOKEN_EXPIRED";
    public const string AuthTokenInvalid = "AUTH_TOKEN_INVALID";
    public const string AuthRefreshTokenExpired = "AUTH_REFRESH_TOKEN_EXPIRED";
    public const string AuthRefreshTokenRevoked = "AUTH_REFRESH_TOKEN_REVOKED";
    public const string AuthAccountBanned = "AUTH_ACCOUNT_BANNED";
    public const string AuthAccountDisabled = "AUTH_ACCOUNT_DISABLED";
    public const string AuthDevLoginDisabled = "AUTH_DEV_LOGIN_DISABLED";
    public const string AuthSteamMockOnly = "AUTH_STEAM_MOCK_ONLY";
    public const string AuthEosMockOnly = "AUTH_EOS_MOCK_ONLY";
    public const string AuthWeChatMockOnly = "AUTH_WECHAT_MOCK_ONLY";
    public const string AuthWeChatInvalidCode = "AUTH_WECHAT_INVALID_CODE";
    public const string AuthTooManyFailedAttempts = "AUTH_TOO_MANY_FAILED_ATTEMPTS";
    public const string AuthAccountLocked = "AUTH_ACCOUNT_LOCKED";

    public const string PlayerNicknameTaken = "PLAYER_NICKNAME_TAKEN";
    public const string PlayerNicknameCooldown = "PLAYER_NICKNAME_COOLDOWN";
    public const string PlayerNotFound = "PLAYER_NOT_FOUND";
    public const string PlayerInvalidNickname = "PLAYER_INVALID_NICKNAME";

    public const string ConfigNotFound = "CONFIG_NOT_FOUND";
    public const string ConfigInvalidJson = "CONFIG_INVALID_JSON";
    public const string ConfigKeyNotAllowed = "CONFIG_KEY_NOT_ALLOWED";
    public const string ConfigAlreadyPublished = "CONFIG_ALREADY_PUBLISHED";

    public const string RoomNotFound = "ROOM_NOT_FOUND";
    public const string RoomFull = "ROOM_FULL";
    public const string RoomAlreadyJoined = "ROOM_ALREADY_JOINED";
    public const string RoomNotOwner = "ROOM_NOT_OWNER";
    public const string RoomNotReady = "ROOM_NOT_READY";
    public const string RoomInProgress = "ROOM_IN_PROGRESS";
    public const string RoomPasswordInvalid = "ROOM_PASSWORD_INVALID";

    public const string MatchTicketNotFound = "MATCH_TICKET_NOT_FOUND";
    public const string MatchTicketTimeout = "MATCH_TICKET_TIMEOUT";
    public const string MatchTicketAlreadyMatched = "MATCH_TICKET_ALREADY_MATCHED";

    public const string SessionNotFound = "SESSION_NOT_FOUND";
    public const string SessionInvalidState = "SESSION_INVALID_STATE";
    public const string SessionPlayerNotInSession = "SESSION_PLAYER_NOT_IN_SESSION";
    public const string SessionServerNotReady = "SESSION_SERVER_NOT_READY";
    public const string SessionTokenInvalid = "SESSION_TOKEN_INVALID";
    public const string SessionReconnectTokenExpired = "SESSION_RECONNECT_TOKEN_EXPIRED";

    public const string ServerNotFound = "SERVER_NOT_FOUND";
    public const string GameServerNotFound = "GAME_SERVER_NOT_FOUND";
    public const string ServerAllocationFailed = "SERVER_ALLOCATION_FAILED";
    public const string ServerPortExhausted = "SERVER_PORT_EXHAUSTED";
    public const string ServerTimeout = "SERVER_TIMEOUT";
    public const string ServerHeartbeatTimeout = "SERVER_HEARTBEAT_TIMEOUT";

    public const string MatchResultAlreadySettled = "MATCH_RESULT_ALREADY_SETTLED";
    public const string MatchResultNotFound = "MATCH_RESULT_NOT_FOUND";
    public const string MatchResultInvalidState = "MATCH_RESULT_INVALID_STATE";
    public const string MatchResultPlayerNotInSession = "MATCH_RESULT_PLAYER_NOT_IN_SESSION";

    public const string InventoryItemNotFound = "INVENTORY_ITEM_NOT_FOUND";
    public const string InventoryInsufficientQuantity = "INVENTORY_INSUFFICIENT_QUANTITY";
    public const string InventoryDuplicateGrant = "INVENTORY_DUPLICATE_GRANT";

    public const string AdminNotFound = "ADMIN_NOT_FOUND";
    public const string AdminInvalidCredentials = "ADMIN_INVALID_CREDENTIALS";
    public const string AdminAccountLocked = "ADMIN_ACCOUNT_LOCKED";

    public const string ValidationError = "VALIDATION_ERROR";
    public const string InternalError = "INTERNAL_ERROR";
}