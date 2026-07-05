/*
中文阅读说明：
- 所属应用：DBA_GameBackend 共享契约。
- 文件职责：定义角色进入对局时的冻结构建摘要 DTO。
- 阅读重点：Zodiac + PrimaryElement 决定 FixedSkillGroupId，FiveCamp 只用于表现包。
- 修改提示：如需扩展构建字段，保持 Dedicated Server 和客户端 DTO 同步。
*/

namespace Game.Shared.Contracts.Character;

public record CharacterBuildSummaryDto(
    string Zodiac,
    string PrimaryElement,
    string FiveCamp,
    string FixedSkillGroupId);

public static class CharacterBuildRules
{
    public static CharacterBuildSummaryDto BuildSummary(string? zodiac, string? primaryElement, string? fiveCamp)
    {
        var normalizedZodiac = NormalizeChoice(zodiac, "Rat");
        var normalizedPrimaryElement = NormalizeChoice(primaryElement, "Water");

        return new CharacterBuildSummaryDto(
            normalizedZodiac,
            normalizedPrimaryElement,
            NormalizeChoice(fiveCamp, "East"),
            BuildFixedSkillGroupId(normalizedZodiac, normalizedPrimaryElement));
    }

    public static string BuildFixedSkillGroupId(string? zodiac, string? primaryElement)
    {
        return $"{NormalizeChoice(zodiac, "Rat")}_{NormalizeChoice(primaryElement, "Water")}";
    }

    public static string NormalizeChoice(string? value, string fallback)
    {
        if (string.IsNullOrWhiteSpace(value))
        {
            return fallback;
        }

        var trimmed = value.Trim();
        return trimmed.Equals("None", StringComparison.OrdinalIgnoreCase)
            ? fallback
            : trimmed;
    }
}
