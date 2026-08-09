using System.Text.Json;
using Game.Application.Characters;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Contracts.Character;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using Npgsql;

namespace Game.Infrastructure.Database.Characters;

/** 单一角色持久化实现：新 CharacterService 与旧账号兼容端点共享同一 characters 表。 */
public sealed class EfCharacterRosterStore(
    GameDbContext db,
    CharacterCreationOptions options,
    TimeProvider timeProvider) : ICharacterRosterStore
{
    private static readonly JsonSerializerOptions JsonOptions = new(JsonSerializerDefaults.Web);

    public async Task<IReadOnlyList<CharacterRecord>> GetRosterAsync(Guid accountId, Guid serverId, CancellationToken cancellationToken = default) =>
        (await db.PlayerCharacters.AsNoTracking()
            .Include(x => x.Appearance).Include(x => x.Progress)
            .Where(x => x.PlayerId == accountId && x.ServerId == serverId && !x.IsDeleted)
            .OrderByDescending(x => x.IsSelected).ThenByDescending(x => x.LastUsedAt)
            .ToListAsync(cancellationToken)).Select(ToRecord).ToArray();

    public async Task<CharacterRecord?> GetAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default)
    {
        var row = await db.PlayerCharacters.AsNoTracking().Include(x => x.Appearance).Include(x => x.Progress)
            .SingleOrDefaultAsync(x => x.PlayerId == accountId && x.Id == characterId && !x.IsDeleted, cancellationToken);
        return row is null ? null : ToRecord(row);
    }

    public async Task<CharacterRecord?> GetByIdempotencyKeyAsync(Guid accountId, Guid serverId, string idempotencyKey, CancellationToken cancellationToken = default)
    {
        var row = await db.PlayerCharacters.AsNoTracking().Include(x => x.Appearance).Include(x => x.Progress)
            .SingleOrDefaultAsync(x => x.PlayerId == accountId && x.ServerId == serverId && x.CreationIdempotencyKey == idempotencyKey, cancellationToken);
        return row is null ? null : ToRecord(row);
    }

    public async Task<CharacterRecord?> CreateAsync(CreateCharacterCommand command, string normalizedName, CancellationToken cancellationToken = default)
    {
        var idempotencyKey = string.IsNullOrWhiteSpace(command.IdempotencyKey) ? null : command.IdempotencyKey.Trim();
        if (idempotencyKey is not null)
        {
            var replay = await GetByIdempotencyKeyAsync(command.AccountId, command.ServerId, idempotencyKey, cancellationToken);
            if (replay is not null) return replay;
        }

        var now = timeProvider.GetUtcNow();
        var row = new PlayerCharacter
        {
            Id = Guid.CreateVersion7(), PlayerId = command.AccountId, ServerId = command.ServerId,
            CharacterName = command.Name!.Normalize().Trim(), NormalizedName = normalizedName,
            Zodiac = command.ZodiacType!.Trim(), PrimaryElement = command.ElementType!.Trim(), FiveCamp = command.FiveCampType!.Trim(),
            FixedSkillGroupId = CharacterBuildRules.BuildFixedSkillGroupId(command.ZodiacType, command.ElementType),
            CoreAttributesJson = "{}", Level = options.InitialLevel, IsSelected = false, IsDeleted = false,
            CreationIdempotencyKey = idempotencyKey, CreatedAt = now, LastUsedAt = now,
            Appearance = new CharacterAppearance { RulesVersion = options.RulesVersion, AppearanceJson = JsonSerializer.Serialize(command.Appearance ?? new CharacterAppearancePayload(null, null, null, null), JsonOptions), CreatedAt = now, UpdatedAt = now },
            Progress = new CharacterProgress { Level = options.InitialLevel, Experience = 0, UpdatedAt = now }
        };
        db.PlayerCharacters.Add(row);
        try { await db.SaveChangesAsync(cancellationToken); }
        catch (DbUpdateException exception) when (exception.InnerException is PostgresException { SqlState: PostgresErrorCodes.UniqueViolation })
        {
            db.Entry(row).State = EntityState.Detached;
            return null;
        }
        return ToRecord(row);
    }

    public async Task<CharacterRecord?> SelectAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default)
    {
        var target = await db.PlayerCharacters.SingleOrDefaultAsync(x => x.PlayerId == accountId && x.Id == characterId && !x.IsDeleted, cancellationToken);
        if (target is null) return null;
        var now = timeProvider.GetUtcNow();
        await using var transaction = await db.Database.BeginTransactionAsync(cancellationToken);
        await db.PlayerCharacters.Where(x => x.PlayerId == accountId && x.ServerId == target.ServerId && x.IsSelected)
            .ExecuteUpdateAsync(s => s.SetProperty(x => x.IsSelected, false), cancellationToken);
        target.IsSelected = true; target.LastUsedAt = now; target.UpdatedAt = now;
        await db.SaveChangesAsync(cancellationToken);
        await transaction.CommitAsync(cancellationToken);
        await db.Entry(target).Reference(x => x.Appearance).LoadAsync(cancellationToken);
        await db.Entry(target).Reference(x => x.Progress).LoadAsync(cancellationToken);
        return ToRecord(target);
    }

    public async Task<bool> SoftDeleteAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default)
    {
        var row = await db.PlayerCharacters.SingleOrDefaultAsync(x => x.PlayerId == accountId && x.Id == characterId && !x.IsDeleted, cancellationToken);
        if (row is null) return false;
        var now = timeProvider.GetUtcNow();
        row.IsDeleted = true;
        row.IsSelected = false;
        row.DeletedAt = now;
        row.UpdatedAt = now;
        await db.SaveChangesAsync(cancellationToken);
        return true;
    }

    public Task<bool> ServerExistsAsync(Guid serverId, CancellationToken cancellationToken = default) => db.GameServers.AnyAsync(x => x.Id == serverId && x.Status == "Online", cancellationToken);
    public Task<int> CountActiveAsync(Guid accountId, Guid serverId, CancellationToken cancellationToken = default) => db.PlayerCharacters.CountAsync(x => x.PlayerId == accountId && x.ServerId == serverId && !x.IsDeleted, cancellationToken);

    private static CharacterRecord ToRecord(PlayerCharacter row) => new(row.Id, row.ServerId, row.CharacterName, row.Zodiac, row.PrimaryElement, row.FiveCamp, ReadAppearance(row.Appearance?.AppearanceJson), row.Progress?.Level ?? row.Level, row.IsSelected, row.CreatedAt);
    private static CharacterAppearancePayload ReadAppearance(string? json)
    {
        try { return JsonSerializer.Deserialize<CharacterAppearancePayload>(json ?? "{}", JsonOptions) ?? new CharacterAppearancePayload(null, null, null, null); }
        catch (JsonException) { return new CharacterAppearancePayload(null, null, null, null); }
    }
}
