/*
中文阅读说明：
- 所属应用：DBA_GameBackend Runtime 服务。
- 文件职责：校验 Dedicated Server 上报的玩家入服构建摘要与后端冻结摘要一致。
- 阅读重点：FiveCamp 只做表现包，允许变化；FixedSkillGroupId 必须由 Zodiac + Element 决定。
- 修改提示：如扩展冻结摘要字段，先补测试再修改这里。
*/

using Game.Infrastructure.Database.Entities;
using Game.Shared.Contracts.Character;
using Game.Shared.Contracts.GameServer;
using System.Text.Json;

namespace Game.Api.Services.Runtime;

public readonly record struct RuntimePlayerJoinValidationResult(bool IsValid, string? ErrorMessage = null);

public static class RuntimePlayerJoinValidator
{
    public const string BuildSummaryMismatchMessage = "Runtime player build summary does not match the frozen session build.";
    public const string BuildSummaryMissingMessage = "Runtime player build summary is required when the session has a frozen build.";
    public const string FrozenBuildSummaryInvalidMessage = "Frozen session build summary is invalid.";

    public static RuntimePlayerJoinValidationResult ValidateBuildSummary(
        PlayerSession playerSession,
        RuntimePlayerJoinedRequest request)
    {
        var bHasFrozenZodiac = !string.IsNullOrWhiteSpace(playerSession.Zodiac);
        var bHasFrozenPrimaryElement = !string.IsNullOrWhiteSpace(playerSession.PrimaryElement);
        var bHasFrozenFixedSkillGroupId = !string.IsNullOrWhiteSpace(playerSession.FixedSkillGroupId);
        var bHasAnyFrozenBuildSummary = bHasFrozenZodiac || bHasFrozenPrimaryElement || bHasFrozenFixedSkillGroupId;
        var bHasCompleteFrozenBuildSummary = bHasFrozenZodiac && bHasFrozenPrimaryElement && bHasFrozenFixedSkillGroupId;

        if (!bHasAnyFrozenBuildSummary)
        {
            return new RuntimePlayerJoinValidationResult(true);
        }

        if (!bHasCompleteFrozenBuildSummary)
        {
            return new RuntimePlayerJoinValidationResult(false, FrozenBuildSummaryInvalidMessage);
        }

        if (IsNoneChoice(playerSession.Zodiac)
            || IsNoneChoice(playerSession.PrimaryElement)
            || IsNoneChoice(playerSession.FixedSkillGroupId))
        {
            return new RuntimePlayerJoinValidationResult(false, FrozenBuildSummaryInvalidMessage);
        }

        var frozenBuildSummary = NormalizeFrozenBuildSummary(playerSession);
        var expectedFrozenFixedSkillGroupId = CharacterBuildRules.BuildFixedSkillGroupId(
            frozenBuildSummary.Zodiac,
            frozenBuildSummary.PrimaryElement);
        if (!string.Equals(frozenBuildSummary.FixedSkillGroupId, expectedFrozenFixedSkillGroupId, StringComparison.OrdinalIgnoreCase))
        {
            return new RuntimePlayerJoinValidationResult(false, FrozenBuildSummaryInvalidMessage);
        }

        if (IsMissingRequiredChoice(request.Zodiac)
            || IsMissingRequiredChoice(request.PrimaryElement)
            || IsMissingRequiredChoice(request.FixedSkillGroupId))
        {
            return new RuntimePlayerJoinValidationResult(false, BuildSummaryMissingMessage);
        }

        var requestZodiac = CharacterBuildRules.NormalizeChoice(request.Zodiac, frozenBuildSummary.Zodiac);
        var requestElement = CharacterBuildRules.NormalizeChoice(request.PrimaryElement, frozenBuildSummary.PrimaryElement);
        var requestFixedSkillGroupId = CharacterBuildRules.NormalizeChoice(
            request.FixedSkillGroupId,
            CharacterBuildRules.BuildFixedSkillGroupId(requestZodiac, requestElement));
        var bMatchesFrozenIdentity =
            string.Equals(requestZodiac, frozenBuildSummary.Zodiac, StringComparison.OrdinalIgnoreCase)
            && string.Equals(requestElement, frozenBuildSummary.PrimaryElement, StringComparison.OrdinalIgnoreCase)
            && string.Equals(requestFixedSkillGroupId, expectedFrozenFixedSkillGroupId, StringComparison.OrdinalIgnoreCase);

        return bMatchesFrozenIdentity
            ? new RuntimePlayerJoinValidationResult(true)
            : new RuntimePlayerJoinValidationResult(false, BuildSummaryMismatchMessage);
    }

    public static string BuildPlayerJoinedEventPayload(PlayerSession playerSession)
    {
        return JsonSerializer.Serialize(new
        {
            playerId = playerSession.PlayerId,
            team = NormalizeEventValue(playerSession.Team),
            zodiac = NormalizeEventValue(playerSession.Zodiac),
            primaryElement = NormalizeEventValue(playerSession.PrimaryElement),
            fiveCamp = NormalizeEventValue(playerSession.FiveCamp),
            fixedSkillGroupId = NormalizeEventValue(playerSession.FixedSkillGroupId)
        });
    }

    private static string? NormalizeEventValue(string? value)
    {
        return string.IsNullOrWhiteSpace(value)
            ? null
            : value.Trim();
    }

    private static CharacterBuildSummaryDto NormalizeFrozenBuildSummary(PlayerSession playerSession)
    {
        var zodiac = CharacterBuildRules.NormalizeChoice(playerSession.Zodiac, "Rat");
        var primaryElement = CharacterBuildRules.NormalizeChoice(playerSession.PrimaryElement, "Water");
        var fixedSkillGroupId = CharacterBuildRules.NormalizeChoice(
            playerSession.FixedSkillGroupId,
            CharacterBuildRules.BuildFixedSkillGroupId(zodiac, primaryElement));

        return new CharacterBuildSummaryDto(
            zodiac,
            primaryElement,
            CharacterBuildRules.NormalizeChoice(playerSession.FiveCamp, string.Empty),
            fixedSkillGroupId);
    }

    private static bool IsMissingRequiredChoice(string? value)
    {
        return string.IsNullOrWhiteSpace(value)
            || IsNoneChoice(value);
    }

    private static bool IsNoneChoice(string? value)
    {
        return value?.Trim().Equals("None", StringComparison.OrdinalIgnoreCase) == true;
    }
}
