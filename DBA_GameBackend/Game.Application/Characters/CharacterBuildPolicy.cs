/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：统一生成、规范化并校验角色冻结构筑，避免 API、会话和入服链路各自维护默认值。
- 架构约束：默认生肖、元素、阵营和错误文案全部来自配置；本策略不依赖 HTTP、EF Core 或数据库实体。
*/

using Game.Shared.Contracts.Character;
using Game.Shared.Options;

namespace Game.Application.Characters;

public sealed record CharacterBuildSnapshot(
    string? Zodiac,
    string? PrimaryElement,
    string? FiveCamp,
    string? FixedSkillGroupId);

public sealed record CharacterBuildNormalizationResult(
    bool HasBuild,
    bool IsValid,
    CharacterBuildSummaryDto? Build,
    string? ErrorMessage);

public sealed record CharacterBuildValidationResult(bool IsValid, string? ErrorMessage = null);

public interface ICharacterBuildPolicy
{
    CharacterBuildSummaryDto BuildSummary(string? zodiac, string? primaryElement, string? fiveCamp);
    CharacterBuildNormalizationResult NormalizeBuild(CharacterBuildSnapshot snapshot);
    CharacterBuildValidationResult ValidateRuntimeJoin(
        CharacterBuildSnapshot frozenBuild,
        CharacterBuildSnapshot requestedBuild);
}

public sealed class CharacterBuildPolicy(CharacterCreationOptions options) : ICharacterBuildPolicy
{
    public CharacterBuildSummaryDto BuildSummary(
        string? zodiac,
        string? primaryElement,
        string? fiveCamp)
    {
        return CharacterBuildRules.BuildSummary(
            zodiac,
            primaryElement,
            fiveCamp,
            options.DefaultZodiac,
            options.DefaultPrimaryElement,
            options.DefaultFiveCamp);
    }

    public CharacterBuildNormalizationResult NormalizeBuild(CharacterBuildSnapshot snapshot)
    {
        var hasZodiac = !string.IsNullOrWhiteSpace(snapshot.Zodiac);
        var hasPrimaryElement = !string.IsNullOrWhiteSpace(snapshot.PrimaryElement);
        var hasFixedSkillGroupId = !string.IsNullOrWhiteSpace(snapshot.FixedSkillGroupId);
        var hasAnyBuild = hasZodiac || hasPrimaryElement || hasFixedSkillGroupId;
        if (!hasAnyBuild)
        {
            return new CharacterBuildNormalizationResult(false, true, null, null);
        }

        if (!hasZodiac || !hasPrimaryElement || !hasFixedSkillGroupId
            || IsNoneChoice(snapshot.Zodiac)
            || IsNoneChoice(snapshot.PrimaryElement)
            || IsNoneChoice(snapshot.FixedSkillGroupId))
        {
            return Invalid(options.Messages.FrozenBuildSummaryInvalid);
        }

        var zodiac = CharacterBuildRules.NormalizeChoice(snapshot.Zodiac, options.DefaultZodiac);
        var primaryElement = CharacterBuildRules.NormalizeChoice(
            snapshot.PrimaryElement,
            options.DefaultPrimaryElement);
        var fixedSkillGroupId = CharacterBuildRules.NormalizeChoice(snapshot.FixedSkillGroupId, string.Empty);
        var expectedFixedSkillGroupId = CharacterBuildRules.BuildFixedSkillGroupId(zodiac, primaryElement);
        if (!string.Equals(fixedSkillGroupId, expectedFixedSkillGroupId, StringComparison.OrdinalIgnoreCase))
        {
            return Invalid(options.Messages.FrozenBuildSummaryInvalid);
        }

        var build = new CharacterBuildSummaryDto(
            zodiac,
            primaryElement,
            CharacterBuildRules.NormalizeChoice(snapshot.FiveCamp, options.DefaultFiveCamp),
            expectedFixedSkillGroupId);
        return new CharacterBuildNormalizationResult(true, true, build, null);
    }

    public CharacterBuildValidationResult ValidateRuntimeJoin(
        CharacterBuildSnapshot frozenBuild,
        CharacterBuildSnapshot requestedBuild)
    {
        var normalizedFrozen = NormalizeBuild(frozenBuild);
        if (!normalizedFrozen.IsValid)
        {
            return new CharacterBuildValidationResult(false, normalizedFrozen.ErrorMessage);
        }

        if (!normalizedFrozen.HasBuild)
        {
            return new CharacterBuildValidationResult(true);
        }

        if (IsMissingRequiredChoice(requestedBuild.Zodiac)
            || IsMissingRequiredChoice(requestedBuild.PrimaryElement)
            || IsMissingRequiredChoice(requestedBuild.FixedSkillGroupId))
        {
            return new CharacterBuildValidationResult(false, options.Messages.BuildSummaryMissing);
        }

        var frozen = normalizedFrozen.Build!;
        var requestedZodiac = CharacterBuildRules.NormalizeChoice(requestedBuild.Zodiac, frozen.Zodiac);
        var requestedPrimaryElement = CharacterBuildRules.NormalizeChoice(
            requestedBuild.PrimaryElement,
            frozen.PrimaryElement);
        var requestedFixedSkillGroupId = CharacterBuildRules.NormalizeChoice(
            requestedBuild.FixedSkillGroupId,
            string.Empty);
        var matchesFrozenIdentity =
            string.Equals(requestedZodiac, frozen.Zodiac, StringComparison.OrdinalIgnoreCase)
            && string.Equals(requestedPrimaryElement, frozen.PrimaryElement, StringComparison.OrdinalIgnoreCase)
            && string.Equals(requestedFixedSkillGroupId, frozen.FixedSkillGroupId, StringComparison.OrdinalIgnoreCase);

        return matchesFrozenIdentity
            ? new CharacterBuildValidationResult(true)
            : new CharacterBuildValidationResult(false, options.Messages.BuildSummaryMismatch);
    }

    private static CharacterBuildNormalizationResult Invalid(string message)
    {
        return new CharacterBuildNormalizationResult(true, false, null, message);
    }

    private static bool IsMissingRequiredChoice(string? value)
    {
        return string.IsNullOrWhiteSpace(value) || IsNoneChoice(value);
    }

    private static bool IsNoneChoice(string? value)
    {
        return value?.Trim().Equals("None", StringComparison.OrdinalIgnoreCase) == true;
    }
}
