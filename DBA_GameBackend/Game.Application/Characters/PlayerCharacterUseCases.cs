/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：编排玩家角色列表、创建和选择用例，不依赖 HTTP、EF Core 或数据库实体。
- 架构约束：角色构筑由共享规则生成，初始数值来自配置，持久化通过 IPlayerCharacterStore 端口完成。
*/

using System.Text.RegularExpressions;
using Game.Shared.Contracts.Character;
using Game.Shared.Options;

namespace Game.Application.Characters;

public sealed record CharacterCoreAttributes(
    float MaxHealth,
    float AttackPower,
    float Defense,
    float MoveSpeed,
    float MaxEnergy,
    float EnergyRegen,
    float CriticalRate,
    float CriticalMultiplier);

public sealed record PlayerCharacterSummary(
    Guid CharacterId,
    string CharacterName,
    string Zodiac,
    string PrimaryElement,
    string FiveCamp,
    string FixedSkillGroupId,
    CharacterCoreAttributes CoreAttributes,
    int Level,
    DateTimeOffset CreatedAt,
    DateTimeOffset LastUsedAt);

public sealed record PlayerCharacterRoster(
    IReadOnlyList<PlayerCharacterSummary> Characters,
    Guid? SelectedCharacterId);

public sealed record NewPlayerCharacter(
    Guid CharacterId,
    Guid PlayerId,
    string CharacterName,
    CharacterBuildSummaryDto Build,
    CharacterCoreAttributes CoreAttributes,
    int Level,
    DateTimeOffset CreatedAt);

public sealed record CreatePlayerCharacterCommand(
    Guid PlayerId,
    string? CharacterName,
    string? Zodiac,
    string? PrimaryElement,
    string? FiveCamp);

public sealed record CharacterUseCaseResult<T>(
    bool Success,
    string ErrorCode,
    string ErrorMessage,
    T? Value)
    where T : class;

public interface IPlayerCharacterStore
{
    Task<PlayerCharacterRoster> GetRosterAsync(Guid playerId, CancellationToken cancellationToken = default);
    Task<PlayerCharacterSummary?> TryCreateAsync(NewPlayerCharacter character, CancellationToken cancellationToken = default);
    Task<PlayerCharacterSummary?> TrySelectAsync(Guid playerId, Guid characterId, CancellationToken cancellationToken = default);
}

public interface IGetPlayerCharactersUseCase
{
    Task<CharacterUseCaseResult<PlayerCharacterRoster>> ExecuteAsync(
        Guid playerId,
        CancellationToken cancellationToken = default);
}

public interface ICreatePlayerCharacterUseCase
{
    Task<CharacterUseCaseResult<PlayerCharacterSummary>> ExecuteAsync(
        CreatePlayerCharacterCommand command,
        CancellationToken cancellationToken = default);
}

public interface ISelectPlayerCharacterUseCase
{
    Task<CharacterUseCaseResult<PlayerCharacterSummary>> ExecuteAsync(
        Guid playerId,
        string? characterId,
        CancellationToken cancellationToken = default);
}

public sealed class GetPlayerCharactersUseCase(
    IPlayerCharacterStore store,
    CharacterCreationOptions options) : IGetPlayerCharactersUseCase
{
    public async Task<CharacterUseCaseResult<PlayerCharacterRoster>> ExecuteAsync(
        Guid playerId,
        CancellationToken cancellationToken = default)
    {
        if (playerId == Guid.Empty)
        {
            return Failure<PlayerCharacterRoster>("CHARACTER_PLAYER_INVALID", options.Messages.InvalidPlayer);
        }

        var roster = await store.GetRosterAsync(playerId, cancellationToken);
        return Success(roster);
    }

    private static CharacterUseCaseResult<T> Success<T>(T value) where T : class =>
        new(true, string.Empty, string.Empty, value);

    private static CharacterUseCaseResult<T> Failure<T>(string code, string message) where T : class =>
        new(false, code, message, null);
}

public sealed class CreatePlayerCharacterUseCase : ICreatePlayerCharacterUseCase
{
    private readonly IPlayerCharacterStore _store;
    private readonly CharacterCreationOptions _options;
    private readonly TimeProvider _timeProvider;
    private readonly ICharacterBuildPolicy _characterBuildPolicy;
    private readonly Regex _nameRegex;

    public CreatePlayerCharacterUseCase(
        IPlayerCharacterStore store,
        CharacterCreationOptions options,
        TimeProvider timeProvider,
        ICharacterBuildPolicy characterBuildPolicy)
    {
        _store = store;
        _options = options;
        _timeProvider = timeProvider;
        _characterBuildPolicy = characterBuildPolicy;
        _nameRegex = new Regex(options.NamePattern, RegexOptions.CultureInvariant, TimeSpan.FromMilliseconds(250));
    }

    public async Task<CharacterUseCaseResult<PlayerCharacterSummary>> ExecuteAsync(
        CreatePlayerCharacterCommand command,
        CancellationToken cancellationToken = default)
    {
        if (command.PlayerId == Guid.Empty)
        {
            return Failure<PlayerCharacterSummary>("CHARACTER_PLAYER_INVALID", "玩家身份无效。");
        }

        var characterName = NormalizeCharacterName(command.CharacterName);
        if (!_nameRegex.IsMatch(characterName))
        {
            return Failure<PlayerCharacterSummary>(
                "CHARACTER_NAME_INVALID",
                _options.Messages.InvalidName);
        }

        var build = _characterBuildPolicy.BuildSummary(
            command.Zodiac,
            command.PrimaryElement,
            command.FiveCamp);
        var attributes = _options.CoreAttributes;
        var character = new NewPlayerCharacter(
            Guid.NewGuid(),
            command.PlayerId,
            characterName,
            build,
            new CharacterCoreAttributes(
                attributes.MaxHealth,
                attributes.AttackPower,
                attributes.Defense,
                attributes.MoveSpeed,
                attributes.MaxEnergy,
                attributes.EnergyRegen,
                attributes.CriticalRate,
                attributes.CriticalMultiplier),
            _options.InitialLevel,
            _timeProvider.GetUtcNow());

        var created = await _store.TryCreateAsync(character, cancellationToken);
        return created is null
            ? Failure<PlayerCharacterSummary>("CHARACTER_NAME_DUPLICATE", _options.Messages.DuplicateName)
            : Success(created);
    }

    private string NormalizeCharacterName(string? characterName)
    {
        if (!string.IsNullOrWhiteSpace(characterName))
        {
            return characterName.Trim();
        }

        var suffix = Guid.NewGuid().ToString("N")[.._options.GeneratedNameSuffixLength];
        return $"{_options.GeneratedNamePrefix}{suffix}";
    }

    private static CharacterUseCaseResult<T> Success<T>(T value) where T : class =>
        new(true, string.Empty, string.Empty, value);

    private static CharacterUseCaseResult<T> Failure<T>(string code, string message) where T : class =>
        new(false, code, message, null);
}

public sealed class SelectPlayerCharacterUseCase(
    IPlayerCharacterStore store,
    CharacterCreationOptions options) : ISelectPlayerCharacterUseCase
{
    public async Task<CharacterUseCaseResult<PlayerCharacterSummary>> ExecuteAsync(
        Guid playerId,
        string? characterId,
        CancellationToken cancellationToken = default)
    {
        if (playerId == Guid.Empty)
        {
            return Failure<PlayerCharacterSummary>("CHARACTER_PLAYER_INVALID", options.Messages.InvalidPlayer);
        }

        if (!Guid.TryParse(characterId, out var parsedCharacterId))
        {
            return Failure<PlayerCharacterSummary>("CHARACTER_ID_INVALID", options.Messages.InvalidCharacterId);
        }

        var selected = await store.TrySelectAsync(playerId, parsedCharacterId, cancellationToken);
        return selected is null
            ? Failure<PlayerCharacterSummary>("CHARACTER_NOT_FOUND", options.Messages.CharacterNotFound)
            : Success(selected);
    }

    private static CharacterUseCaseResult<T> Success<T>(T value) where T : class =>
        new(true, string.Empty, string.Empty, value);

    private static CharacterUseCaseResult<T> Failure<T>(string code, string message) where T : class =>
        new(false, code, message, null);
}
