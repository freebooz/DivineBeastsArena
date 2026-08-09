/*
中文阅读说明：
- 本文件为既有 PostgreSQL 会话准入表的 Redis 短期索引，不创建第二套 GameTicket。
- 票据明文只在签发到客户端、或 Dedicated Server 提交消费时短暂出现；Redis 键和数据库均只保存哈希。
- Redis 的 Lua GET + DEL 用于快速、原子地拒绝同一票据的第二次消费；随后 PostgreSQL 仍执行最终权威的条件更新。
*/

using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using Game.Application.Sessions;
using StackExchange.Redis;

namespace Game.Infrastructure.Redis;

/// <summary>
/// 记录同一张既有 JoinTicket 的短期绑定。该接口不签发、不暴露也不记录票据明文。
/// </summary>
public interface IGameTicketRedisRegistry
{
    Task<bool> RecordIssuedAsync(IssuedSessionConnection connection, CancellationToken cancellationToken = default);
    Task<bool> TryConsumeAsync(ConsumeJoinTicketCommand command, CancellationToken cancellationToken = default);
}

/// <summary>
/// Redis 票据索引实现。键的 TTL 与签发凭证到期时间一致，避免 Redis 留存过期的入服上下文。
/// </summary>
public sealed class GameTicketRedisRegistry(IRedisConnectionFactory connectionFactory) : IGameTicketRedisRegistry
{
    private const string KeyPrefix = "dba:game-ticket:";
    private const string AtomicGetAndDeleteScript = "local value = redis.call('GET', KEYS[1]); if value then redis.call('DEL', KEYS[1]); end; return value;";
    private static readonly JsonSerializerOptions SerializerOptions = new(JsonSerializerDefaults.Web);

    public async Task<bool> RecordIssuedAsync(IssuedSessionConnection connection, CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (string.IsNullOrWhiteSpace(connection.JoinTicket) || connection.JoinTicketExpiresAt <= DateTimeOffset.UtcNow)
        {
            return false;
        }

        var binding = new GameTicketBinding(
            connection.PlayerId,
            connection.CharacterId,
            connection.SessionId,
            connection.ServerInstanceId,
            connection.BuildId);
        var ttl = connection.JoinTicketExpiresAt - DateTimeOffset.UtcNow;
        return await connectionFactory.GetDatabase().StringSetAsync(
            BuildKey(connection.JoinTicket),
            JsonSerializer.Serialize(binding, SerializerOptions),
            ttl,
            When.Always).ConfigureAwait(false);
    }

    public async Task<bool> TryConsumeAsync(ConsumeJoinTicketCommand command, CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (string.IsNullOrWhiteSpace(command.JoinTicket))
        {
            return false;
        }

        // 单 Redis 键上的 Lua GET + DEL 是原子操作：第二次请求读取到空值而失败。
        var raw = await connectionFactory.GetDatabase().ScriptEvaluateAsync(
            AtomicGetAndDeleteScript,
            [BuildKey(command.JoinTicket)]).ConfigureAwait(false);
        var bindingJson = raw.ToString();
        if (string.IsNullOrEmpty(bindingJson))
        {
            return false;
        }

        GameTicketBinding? binding;
        try
        {
            binding = JsonSerializer.Deserialize<GameTicketBinding>(bindingJson, SerializerOptions);
        }
        catch (JsonException)
        {
            return false;
        }

        // 不信任 Redis 中的序列化内容；消费请求仍必须与所有绑定字段精确匹配。
        return binding is not null
            && binding.AccountId == command.AccountId
            && binding.CharacterId == command.CharacterId
            && binding.SessionId == command.SessionId
            && binding.ServerInstanceId == command.ServerInstanceId
            && string.Equals(binding.BuildId, command.BuildId.Trim(), StringComparison.Ordinal);
    }

    private static RedisKey BuildKey(string plaintextTicket)
    {
        var hash = Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(plaintextTicket))).ToLowerInvariant();
        return KeyPrefix + hash;
    }

    private sealed record GameTicketBinding(
        Guid AccountId,
        Guid CharacterId,
        Guid SessionId,
        Guid ServerInstanceId,
        string BuildId);
}
