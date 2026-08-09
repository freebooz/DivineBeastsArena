/*
中文阅读说明：
- 所属应用：DBA_GameBackend 共享配置层。
- 文件职责：集中校验生产/运行必需配置，避免 API 与 Worker 启动后才暴露缺失配置。
- 阅读重点：每个 Validate 方法对应一个配置段，异常信息保持为可直接定位的配置键。
- 修改提示：新增必需配置时优先补充这里，并同步测试和 .env.example。
*/

namespace Game.Shared.Options;

using System.Linq;
using System.Text.RegularExpressions;

public static class RequiredOptionsValidator
{
	public static void ValidateVillageSession(VillageSessionOptions options)
	{
		if (string.IsNullOrWhiteSpace(options.Mode)
			|| string.IsNullOrWhiteSpace(options.MapId)
			|| string.IsNullOrWhiteSpace(options.Region))
		{
			throw new InvalidOperationException("VillageSession 的模式、地图和区域必须完整配置。");
		}

		if (options.MaxPlayers is < 1 or > 128)
		{
			throw new InvalidOperationException("VillageSession:MaxPlayers 必须介于 1 和 128 之间。");
		}
	}

	public static void ValidateSessionAdmission(SessionAdmissionOptions options)
	{
		if (options.TokenByteLength is < 16 or > 128)
		{
			throw new InvalidOperationException("SessionAdmission:TokenByteLength 必须介于 16 和 128 之间。");
		}

		if (options.ConnectionTokenLifetimeMinutes <= 0
			|| options.ReconnectTokenLifetimeMinutes <= 0
			|| options.ProvisionalTokenLifetimeMinutes <= 0)
		{
			throw new InvalidOperationException("SessionAdmission 的凭证有效期必须大于 0 分钟。");
		}

		if (string.IsNullOrWhiteSpace(options.MatchMapId))
		{
			throw new InvalidOperationException("必须配置 SessionAdmission:MatchMapId。");
		}

		if (options.MatchMaxPlayers is < 1 or > 128)
		{
			throw new InvalidOperationException("SessionAdmission:MatchMaxPlayers 必须介于 1 和 128 之间。");
		}
	}

	public static void ValidateCharacterCreation(CharacterCreationOptions options)
	{
		if (string.IsNullOrWhiteSpace(options.GeneratedNamePrefix))
		{
			throw new InvalidOperationException("必须配置 CharacterCreation:GeneratedNamePrefix。");
		}

		if (options.GeneratedNameSuffixLength is < 1 or > 32)
		{
			throw new InvalidOperationException("CharacterCreation:GeneratedNameSuffixLength 必须介于 1 和 32 之间。");
		}

		if (string.IsNullOrWhiteSpace(options.NamePattern))
		{
			throw new InvalidOperationException("必须配置 CharacterCreation:NamePattern。");
		}

		if (string.IsNullOrWhiteSpace(options.DefaultZodiac)
			|| string.IsNullOrWhiteSpace(options.DefaultPrimaryElement)
			|| string.IsNullOrWhiteSpace(options.DefaultFiveCamp))
		{
			throw new InvalidOperationException("CharacterCreation 的默认生肖、元素和阵营必须完整配置。");
		}

		try
		{
			_ = new Regex(options.NamePattern, RegexOptions.CultureInvariant, TimeSpan.FromMilliseconds(250));
		}
		catch (ArgumentException exception)
		{
			throw new InvalidOperationException("CharacterCreation:NamePattern 不是有效的正则表达式。", exception);
		}

		if (options.InitialLevel <= 0)
		{
			throw new InvalidOperationException("CharacterCreation:InitialLevel 必须大于 0。");
		}

		var attributes = options.CoreAttributes;
		if (attributes.MaxHealth <= 0
			|| attributes.AttackPower < 0
			|| attributes.Defense < 0
			|| attributes.MoveSpeed <= 0
			|| attributes.MaxEnergy < 0
			|| attributes.EnergyRegen < 0
			|| attributes.CriticalRate < 0
			|| attributes.CriticalMultiplier <= 0)
		{
			throw new InvalidOperationException("CharacterCreation:CoreAttributes 包含非法初始属性。");
		}

		var messages = options.Messages;
		if (string.IsNullOrWhiteSpace(messages.InvalidPlayer)
			|| string.IsNullOrWhiteSpace(messages.InvalidName)
			|| string.IsNullOrWhiteSpace(messages.DuplicateName)
			|| string.IsNullOrWhiteSpace(messages.InvalidCharacterId)
			|| string.IsNullOrWhiteSpace(messages.CharacterNotFound)
			|| string.IsNullOrWhiteSpace(messages.BuildSummaryMismatch)
			|| string.IsNullOrWhiteSpace(messages.BuildSummaryMissing)
			|| string.IsNullOrWhiteSpace(messages.FrozenBuildSummaryInvalid))
		{
			throw new InvalidOperationException("CharacterCreation:Messages 必须完整配置中文错误文案。");
		}
	}

	public static void ValidateAuthenticationPolicy(AuthenticationPolicyOptions options)
	{
		if (options.MinimumPasswordLength is < 6 or > 128)
		{
			throw new InvalidOperationException("AuthenticationPolicy:MinimumPasswordLength 必须介于 6 和 128 之间。");
		}

		if (string.IsNullOrWhiteSpace(options.ActiveAccountStatus)
			|| string.IsNullOrWhiteSpace(options.GuestAccountType)
			|| string.IsNullOrWhiteSpace(options.RegisteredAccountType)
			|| string.IsNullOrWhiteSpace(options.GuestDisplayNamePrefix)
			|| string.IsNullOrWhiteSpace(options.InitialPlayerSettingsJson))
		{
			throw new InvalidOperationException("AuthenticationPolicy 的账号类型、状态、访客名称和初始设置必须完整配置。");
		}

		if (options.GuestDisplayNameSuffixLength is < 1 or > 32)
		{
			throw new InvalidOperationException("AuthenticationPolicy:GuestDisplayNameSuffixLength 必须介于 1 和 32 之间。");
		}

		if (options.InitialPlayerLevel <= 0 || options.InitialPlayerExperience < 0)
		{
			throw new InvalidOperationException("AuthenticationPolicy 的初始玩家等级和经验配置无效。");
		}

		var messages = options.Messages;
		if (string.IsNullOrWhiteSpace(messages.PasswordFieldsRequired)
			|| string.IsNullOrWhiteSpace(messages.EmailAndPasswordRequired)
			|| string.IsNullOrWhiteSpace(messages.PasswordTooShort)
			|| string.IsNullOrWhiteSpace(messages.AccountNotFound)
			|| string.IsNullOrWhiteSpace(messages.AccountBanned)
			|| string.IsNullOrWhiteSpace(messages.PasswordCredentialMissing)
			|| string.IsNullOrWhiteSpace(messages.OldPasswordIncorrect)
			|| string.IsNullOrWhiteSpace(messages.PasswordUnchanged)
			|| string.IsNullOrWhiteSpace(messages.ResetServiceUnavailable)
			|| string.IsNullOrWhiteSpace(messages.ResetTokenInvalid)
			|| string.IsNullOrWhiteSpace(messages.PasswordChanged)
			|| string.IsNullOrWhiteSpace(messages.PasswordResetCompleted)
			|| string.IsNullOrWhiteSpace(messages.UsernameOrEmailRequired)
			|| string.IsNullOrWhiteSpace(messages.UsernameTaken)
			|| string.IsNullOrWhiteSpace(messages.IdentityConflict)
			|| string.IsNullOrWhiteSpace(messages.InvalidOnboarding)
			|| string.IsNullOrWhiteSpace(messages.InvalidCredentials)
			|| string.IsNullOrWhiteSpace(messages.DevelopmentLoginDisabled)
			|| string.IsNullOrWhiteSpace(messages.RefreshTokenInvalid)
			|| string.IsNullOrWhiteSpace(messages.SteamLoginUnavailable)
			|| string.IsNullOrWhiteSpace(messages.EosLoginUnavailable)
			|| string.IsNullOrWhiteSpace(messages.WeChatLoginUnavailable))
		{
			throw new InvalidOperationException("AuthenticationPolicy:Messages 必须完整配置中文响应文案。");
		}
	}

	/** 验证自动游戏名的运营字符库，避免服务端在首次登录时生成非汉字或长度不符合产品约束的名称。 */
	public static void ValidatePlayerGameName(PlayerGameNameOptions options)
	{
		if (options.MinimumHanCharacters != 3 || options.MaximumHanCharacters != 5)
		{
			throw new InvalidOperationException("PlayerGameName 的汉字长度必须固定为 3 至 5。");
		}
		if (options.GenerationAttempts is < 1 or > 256)
		{
			throw new InvalidOperationException("PlayerGameName:GenerationAttempts 必须介于 1 和 256 之间。");
		}
		if (options.Surnames.Length == 0 || options.GivenNameCharacters.Length == 0
			|| options.Surnames.Any(name => !IsSingleHanCharacter(name))
			|| options.GivenNameCharacters.Any(name => !IsSingleHanCharacter(name)))
		{
			throw new InvalidOperationException("PlayerGameName 的姓氏和名字字库必须均为单个汉字。");
		}
	}

	private static bool IsSingleHanCharacter(string? value) =>
		value is { Length: 1 } && value[0] is >= '\u4E00' and <= '\u9FFF';

    public static void ValidateDatabase(DatabaseOptions options)
    {
        if (string.IsNullOrWhiteSpace(options.ConnectionString))
        {
            throw new InvalidOperationException("Database:ConnectionString must be configured.");
        }
    }

    public static void ValidateRedis(RedisOptions options)
    {
        if (string.IsNullOrWhiteSpace(options.ConnectionString))
        {
            throw new InvalidOperationException("Redis:ConnectionString must be configured.");
        }
    }

    public static void ValidateJwt(JwtOptions options)
    {
        if (string.IsNullOrWhiteSpace(options.Secret) || options.Secret.Length < 32)
        {
            throw new InvalidOperationException("Jwt:Secret must be configured with at least 32 characters.");
        }

        if (string.IsNullOrWhiteSpace(options.Issuer))
        {
            throw new InvalidOperationException("Jwt:Issuer must be configured.");
        }

        if (string.IsNullOrWhiteSpace(options.Audience))
        {
            throw new InvalidOperationException("Jwt:Audience must be configured.");
        }
    }

    public static void ValidateInternalApiKey(string? key)
    {
        if (string.IsNullOrWhiteSpace(key) || key.Length < 32)
        {
            throw new InvalidOperationException("InternalApi:Key must be configured with at least 32 characters.");
        }
    }

    public static void ValidateDedicatedServerOrchestration(
        DedicatedServerOrchestrationOptions options,
        bool isProduction = false)
    {
        if (string.IsNullOrWhiteSpace(options.ServerMode))
        {
            throw new InvalidOperationException("GameServerManager:ServerMode must be configured.");
        }

        if (!options.ServerMode.Equals("LocalProcess", StringComparison.OrdinalIgnoreCase) &&
            !options.ServerMode.Equals("Docker", StringComparison.OrdinalIgnoreCase) &&
            !options.ServerMode.Equals("External", StringComparison.OrdinalIgnoreCase))
        {
            throw new InvalidOperationException("GameServerManager:ServerMode must be LocalProcess, Docker, or External.");
        }

        if (string.IsNullOrWhiteSpace(options.PublicIp))
        {
            throw new InvalidOperationException("GameServerManager:PublicIp must be configured.");
        }

        if (options.PortRangeStart <= 0 || options.PortRangeEnd <= 0 || options.PortRangeEnd < options.PortRangeStart)
        {
            throw new InvalidOperationException("GameServerManager port range must be valid.");
        }

        if (options.MaxServersPerMachine <= 0)
        {
            throw new InvalidOperationException("GameServerManager:MaxServersPerMachine must be greater than 0.");
        }

        if (options.StartupTimeoutSeconds < 30 ||
            options.HeartbeatTimeoutSeconds < 30 ||
            options.IdleTimeoutSeconds < 60)
        {
            throw new InvalidOperationException("GameServerManager timeout values are below supported minimums.");
        }

        if (options.ServerMode.Equals("Docker", StringComparison.OrdinalIgnoreCase) &&
            string.IsNullOrWhiteSpace(options.UeServerImage))
        {
            throw new InvalidOperationException("GameServerManager:UeServerImage must be configured in Docker mode.");
        }

        if (isProduction && options.AllowMockServerAllocation)
        {
            throw new InvalidOperationException("GameServerManager:AllowMockServerAllocation must be false in Production.");
        }

        if (isProduction && options.ServerMode.Equals("LocalProcess", StringComparison.OrdinalIgnoreCase))
        {
            if (string.IsNullOrWhiteSpace(options.UeServerExecutablePath))
            {
                throw new InvalidOperationException("GameServerManager:UeServerExecutablePath must be configured in Production LocalProcess mode.");
            }

            if (!File.Exists(options.UeServerExecutablePath))
            {
                throw new InvalidOperationException("GameServerManager:UeServerExecutablePath must point to an existing file in Production LocalProcess mode.");
            }
        }
    }
}

