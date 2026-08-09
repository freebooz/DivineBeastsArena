using System.Text;
using System.Text.RegularExpressions;
using Game.Shared.Options;

namespace Game.Application.Characters;

/** 前后台通用的稳定外观 ID 载体；禁止传递任意客户端资源路径。 */
public sealed record CharacterAppearancePayload(
    IReadOnlyDictionary<string, string>? OptionIds,
    IReadOnlyList<string>? EquipmentVisualIds,
    string? WeaponVisualId,
    string? SkinId);

public sealed record CharacterRecord(
    Guid CharacterId,
    Guid ServerId,
    string Name,
    string ZodiacType,
    string ElementType,
    string FiveCampType,
    CharacterAppearancePayload Appearance,
    int Level,
    bool IsSelected,
    DateTimeOffset CreatedAt);

public sealed record CreateCharacterCommand(
    Guid AccountId,
    Guid ServerId,
    string? Name,
    string? ZodiacType,
    string? ElementType,
    string? FiveCampType,
    CharacterAppearancePayload? Appearance,
    string? IdempotencyKey);

public sealed record CharacterServiceResult<T>(bool Success, string ErrorCode, string ErrorMessage, T? Value)
    where T : class;

public interface ICharacterRosterStore
{
    Task<IReadOnlyList<CharacterRecord>> GetRosterAsync(Guid accountId, Guid serverId, CancellationToken cancellationToken = default);
    Task<CharacterRecord?> GetAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default);
    Task<CharacterRecord?> GetByIdempotencyKeyAsync(Guid accountId, Guid serverId, string idempotencyKey, CancellationToken cancellationToken = default);
    Task<CharacterRecord?> CreateAsync(CreateCharacterCommand command, string normalizedName, CancellationToken cancellationToken = default);
    Task<CharacterRecord?> SelectAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default);
    Task<bool> SoftDeleteAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default);
    Task<bool> ServerExistsAsync(Guid serverId, CancellationToken cancellationToken = default);
    Task<int> CountActiveAsync(Guid accountId, Guid serverId, CancellationToken cancellationToken = default);
}

public interface ICharacterRosterService
{
    Task<CharacterServiceResult<IReadOnlyList<CharacterRecord>>> GetRosterAsync(Guid accountId, Guid serverId, CancellationToken cancellationToken = default);
    Task<CharacterServiceResult<CharacterRecord>> GetAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default);
    Task<CharacterServiceResult<CharacterRecord>> CreateAsync(CreateCharacterCommand command, CancellationToken cancellationToken = default);
    Task<CharacterServiceResult<CharacterRecord>> SelectAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default);
    Task<CharacterServiceResult<object>> DeleteAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default);
}

public sealed class CharacterDefinitionValidator(CharacterCreationOptions options)
{
    private readonly Regex _namePattern = new(options.NamePattern, RegexOptions.CultureInvariant, TimeSpan.FromMilliseconds(250));

    public CharacterServiceResult<object> Validate(CreateCharacterCommand command, out string normalizedName)
    {
        normalizedName = string.Empty;
        if (command.ServerId == Guid.Empty)
        {
            return Fail("SERVER_ID_INVALID", "区服标识无效。");
        }
        if (command.AccountId == Guid.Empty)
        {
            return Fail("ACCOUNT_ID_INVALID", "账号身份无效。");
        }
        if (string.IsNullOrWhiteSpace(command.Name))
        {
            return Fail("CHARACTER_NAME_INVALID", options.Messages.InvalidName);
        }

        var name = command.Name.Normalize(NormalizationForm.FormKC).Trim();
        if (!_namePattern.IsMatch(name))
        {
            return Fail("CHARACTER_NAME_INVALID", options.Messages.InvalidName);
        }
        normalizedName = name.ToUpperInvariant();
        if (options.ReservedNames.Any(x => string.Equals(x.Normalize(NormalizationForm.FormKC).Trim(), name, StringComparison.OrdinalIgnoreCase)))
        {
            return Fail("CHARACTER_NAME_RESERVED", "角色名称为保留词。");
        }
        if (!IsAllowed(options.AllowedZodiacTypes, command.ZodiacType)
            || !IsAllowed(options.AllowedElementTypes, command.ElementType)
            || !IsAllowed(options.AllowedFiveCampTypes, command.FiveCampType))
        {
            return Fail("CHARACTER_BUILD_INVALID", "生肖、元素或五大阵营配置无效。");
        }
        if (!string.IsNullOrWhiteSpace(command.IdempotencyKey) && command.IdempotencyKey.Trim().Length > 128)
        {
            return Fail("IDEMPOTENCY_KEY_INVALID", "幂等键长度无效。");
        }

        var appearance = command.Appearance ?? new CharacterAppearancePayload(null, null, null, null);
        if (!options.AppearanceRules.TryGetValue(command.ZodiacType!.Trim(), out var rule))
        {
            return Fail("APPEARANCE_RULE_MISSING", "该生肖缺少服务端外观规则。 ");
        }
        var allowed = rule.AllowedOptionIds.ToHashSet(StringComparer.Ordinal);
        var optionIds = (appearance.OptionIds?.Values ?? Enumerable.Empty<string>())
            .Concat(appearance.EquipmentVisualIds ?? [])
            .Append(appearance.WeaponVisualId ?? string.Empty)
            .Append(appearance.SkinId ?? string.Empty)
            .Where(x => !string.IsNullOrWhiteSpace(x));
        if (optionIds.Any(x => !allowed.Contains(x.Trim())))
        {
            return Fail("APPEARANCE_OPTION_INVALID", "外观选项不属于所选生肖。");
        }
        return new CharacterServiceResult<object>(true, string.Empty, string.Empty, new object());
    }

    private static bool IsAllowed(IEnumerable<string> values, string? candidate) =>
        !string.IsNullOrWhiteSpace(candidate)
        && values.Any(x => string.Equals(x, candidate.Trim(), StringComparison.OrdinalIgnoreCase));

    private static CharacterServiceResult<object> Fail(string code, string message) => new(false, code, message, null);
}

public sealed class CharacterRosterService(
    ICharacterRosterStore store,
    CharacterDefinitionValidator validator,
    CharacterCreationOptions options) : ICharacterRosterService
{
    public async Task<CharacterServiceResult<IReadOnlyList<CharacterRecord>>> GetRosterAsync(Guid accountId, Guid serverId, CancellationToken cancellationToken = default)
    {
        if (accountId == Guid.Empty || serverId == Guid.Empty) return Fail<IReadOnlyList<CharacterRecord>>("REQUEST_INVALID", "账号或区服标识无效。");
        return Ok(await store.GetRosterAsync(accountId, serverId, cancellationToken));
    }

    public async Task<CharacterServiceResult<CharacterRecord>> GetAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default)
    {
        if (accountId == Guid.Empty || characterId == Guid.Empty) return Fail<CharacterRecord>("REQUEST_INVALID", "账号或角色标识无效。");
        var result = await store.GetAsync(accountId, characterId, cancellationToken);
        return result is null ? Fail<CharacterRecord>("CHARACTER_NOT_FOUND", options.Messages.CharacterNotFound) : Ok(result);
    }

    public async Task<CharacterServiceResult<CharacterRecord>> CreateAsync(CreateCharacterCommand command, CancellationToken cancellationToken = default)
    {
        var validation = validator.Validate(command, out var normalizedName);
        if (!validation.Success) return Fail<CharacterRecord>(validation.ErrorCode, validation.ErrorMessage);
        if (!string.IsNullOrWhiteSpace(command.IdempotencyKey))
        {
            var replay = await store.GetByIdempotencyKeyAsync(command.AccountId, command.ServerId, command.IdempotencyKey.Trim(), cancellationToken);
            if (replay is not null) return Ok(replay);
        }
        if (!await store.ServerExistsAsync(command.ServerId, cancellationToken)) return Fail<CharacterRecord>("SERVER_NOT_FOUND", "区服不存在或不可用。");
        if (await store.CountActiveAsync(command.AccountId, command.ServerId, cancellationToken) >= options.MaxSlotsPerServer) return Fail<CharacterRecord>("CHARACTER_SLOT_LIMIT", "当前区服的角色槽位已满。");
        var created = await store.CreateAsync(command, normalizedName, cancellationToken);
        return created is null ? Fail<CharacterRecord>("CHARACTER_NAME_DUPLICATE", options.Messages.DuplicateName) : Ok(created);
    }

    public async Task<CharacterServiceResult<CharacterRecord>> SelectAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default)
    {
        var selected = await store.SelectAsync(accountId, characterId, cancellationToken);
        return selected is null ? Fail<CharacterRecord>("CHARACTER_NOT_FOUND", options.Messages.CharacterNotFound) : Ok(selected);
    }

    public async Task<CharacterServiceResult<object>> DeleteAsync(Guid accountId, Guid characterId, CancellationToken cancellationToken = default)
    {
        var deleted = await store.SoftDeleteAsync(accountId, characterId, cancellationToken);
        return deleted ? Ok<object>(new object()) : Fail<object>("CHARACTER_NOT_FOUND", options.Messages.CharacterNotFound);
    }

    private static CharacterServiceResult<T> Ok<T>(T value) where T : class => new(true, string.Empty, string.Empty, value);
    private static CharacterServiceResult<T> Fail<T>(string code, string message) where T : class => new(false, code, message, null);
}
