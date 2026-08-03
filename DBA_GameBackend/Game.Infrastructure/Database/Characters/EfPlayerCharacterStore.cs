/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：使用 EF Core 实现玩家角色仓储端口，封装实体映射、唯一约束和角色选择事务。
- 架构约束：API 与应用层不得直接访问 GameDbContext 或 PlayerCharacter 实体。
*/

using System.Text.Json;
using Game.Application.Characters;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;
using Npgsql;

namespace Game.Infrastructure.Database.Characters;

public sealed class EfPlayerCharacterStore(
    GameDbContext db,
    CharacterCreationOptions options,
    TimeProvider timeProvider,
    ILogger<EfPlayerCharacterStore> logger) : IPlayerCharacterStore
{
    private static readonly JsonSerializerOptions JsonOptions = new(JsonSerializerDefaults.Web);

    public async Task<PlayerCharacterRoster> GetRosterAsync(
        Guid playerId,
        CancellationToken cancellationToken = default)
    {
        var rows = await db.PlayerCharacters
            .AsNoTracking()
            .Where(x => x.PlayerId == playerId)
            .OrderByDescending(x => x.IsSelected)
            .ThenByDescending(x => x.LastUsedAt)
            .ThenBy(x => x.CreatedAt)
            .ToListAsync(cancellationToken);

        return new PlayerCharacterRoster(
            rows.Select(ToSummary).ToArray(),
            rows.FirstOrDefault(x => x.IsSelected)?.Id);
    }

    public async Task<PlayerCharacterSummary?> TryCreateAsync(
        NewPlayerCharacter character,
        CancellationToken cancellationToken = default)
    {
        var row = new PlayerCharacter
        {
            Id = character.CharacterId,
            PlayerId = character.PlayerId,
            CharacterName = character.CharacterName,
            Zodiac = character.Build.Zodiac,
            PrimaryElement = character.Build.PrimaryElement,
            FiveCamp = character.Build.FiveCamp,
            FixedSkillGroupId = character.Build.FixedSkillGroupId,
            CoreAttributesJson = JsonSerializer.Serialize(character.CoreAttributes, JsonOptions),
            Level = character.Level,
            IsSelected = false,
            CreatedAt = character.CreatedAt,
            LastUsedAt = character.CreatedAt
        };

        db.PlayerCharacters.Add(row);
        try
        {
            await db.SaveChangesAsync(cancellationToken);
        }
        catch (DbUpdateException exception) when (
            exception.InnerException is PostgresException { SqlState: PostgresErrorCodes.UniqueViolation })
        {
            db.Entry(row).State = EntityState.Detached;
            return null;
        }

        return ToSummary(row);
    }

    public async Task<PlayerCharacterSummary?> TrySelectAsync(
        Guid playerId,
        Guid characterId,
        CancellationToken cancellationToken = default)
    {
        var exists = await db.PlayerCharacters
            .AsNoTracking()
            .AnyAsync(x => x.PlayerId == playerId && x.Id == characterId, cancellationToken);
        if (!exists)
        {
            return null;
        }

        var now = timeProvider.GetUtcNow();
        await using var transaction = await db.Database.BeginTransactionAsync(cancellationToken);

        await db.PlayerCharacters
            .Where(x => x.PlayerId == playerId && x.IsSelected)
            .ExecuteUpdateAsync(
                setters => setters.SetProperty(x => x.IsSelected, false),
                cancellationToken);

        var affectedRows = await db.PlayerCharacters
            .Where(x => x.PlayerId == playerId && x.Id == characterId)
            .ExecuteUpdateAsync(
                setters => setters
                    .SetProperty(x => x.IsSelected, true)
                    .SetProperty(x => x.LastUsedAt, now)
                    .SetProperty(x => x.UpdatedAt, now),
                cancellationToken);
        if (affectedRows != 1)
        {
            await transaction.RollbackAsync(cancellationToken);
            return null;
        }

        await transaction.CommitAsync(cancellationToken);
        var selected = await db.PlayerCharacters
            .AsNoTracking()
            .SingleAsync(x => x.PlayerId == playerId && x.Id == characterId, cancellationToken);
        return ToSummary(selected);
    }

    private PlayerCharacterSummary ToSummary(PlayerCharacter row)
    {
        return new PlayerCharacterSummary(
            row.Id,
            row.CharacterName,
            row.Zodiac,
            row.PrimaryElement,
            row.FiveCamp,
            row.FixedSkillGroupId,
            ReadCoreAttributes(row),
            row.Level,
            row.CreatedAt,
            row.LastUsedAt);
    }

    private CharacterCoreAttributes ReadCoreAttributes(PlayerCharacter row)
    {
        try
        {
            var attributes = JsonSerializer.Deserialize<CharacterCoreAttributes>(row.CoreAttributesJson, JsonOptions);
            if (attributes is not null)
            {
                return attributes;
            }
        }
        catch (JsonException exception)
        {
            logger.LogError(exception, "角色核心属性 JSON 无效，使用配置中的初始属性。角色ID={CharacterId}", row.Id);
        }

        var fallback = options.CoreAttributes;
        return new CharacterCoreAttributes(
            fallback.MaxHealth,
            fallback.AttackPower,
            fallback.Defense,
            fallback.MoveSpeed,
            fallback.MaxEnergy,
            fallback.EnergyRegen,
            fallback.CriticalRate,
            fallback.CriticalMultiplier);
    }
}
