/*
中文阅读说明：
- 所属应用：DBA_GameBackend 共享配置层。
- 文件职责：集中校验生产/运行必需配置，避免 API 与 Worker 启动后才暴露缺失配置。
- 阅读重点：每个 Validate 方法对应一个配置段，异常信息保持为可直接定位的配置键。
- 修改提示：新增必需配置时优先补充这里，并同步测试和 .env.example。
*/

namespace Game.Shared.Options;

public static class RequiredOptionsValidator
{
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

