/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端基础设施。
- 文件职责：读取形如 Jwt__Secret_FILE=/run/secrets/jwt_secret 的环境变量，并把文件内容注入为 Jwt:Secret 配置。
- 阅读重点：这是 Docker secrets、Kubernetes projected secrets、systemd credential 等集中密钥方案的兼容层。
- 修改提示：新增敏感配置时无需改代码，只要按 <配置键>_FILE 命名环境变量即可。
*/

using Microsoft.Extensions.Configuration;

namespace Game.Infrastructure.Configuration;

public static class SecretFileConfigurationExtensions
{
    public static IConfigurationBuilder AddSecretFilesFromEnvironment(this IConfigurationBuilder configuration)
    {
        var secretValues = new Dictionary<string, string?>(StringComparer.OrdinalIgnoreCase);

        foreach (var entry in Environment.GetEnvironmentVariables().Cast<System.Collections.DictionaryEntry>())
        {
            var name = entry.Key?.ToString();
            var filePath = entry.Value?.ToString();
            if (string.IsNullOrWhiteSpace(name) ||
                string.IsNullOrWhiteSpace(filePath) ||
                !name.EndsWith("_FILE", StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            if (!File.Exists(filePath))
            {
                continue;
            }

            var configKey = name[..^"_FILE".Length].Replace("__", ":", StringComparison.Ordinal);
            secretValues[configKey] = File.ReadAllText(filePath).Trim();
        }

        if (secretValues.Count > 0)
        {
            configuration.AddInMemoryCollection(secretValues);
        }

        return configuration;
    }
}
