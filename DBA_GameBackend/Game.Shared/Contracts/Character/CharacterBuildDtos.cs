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
	public static CharacterBuildSummaryDto BuildSummary(
		string? zodiac,
		string? primaryElement,
		string? fiveCamp,
		string defaultZodiac,
		string defaultPrimaryElement,
		string defaultFiveCamp)
	{
		var normalizedZodiac = NormalizeChoice(zodiac, defaultZodiac);
		var normalizedPrimaryElement = NormalizeChoice(primaryElement, defaultPrimaryElement);

		return new CharacterBuildSummaryDto(
			normalizedZodiac,
			normalizedPrimaryElement,
			NormalizeChoice(fiveCamp, defaultFiveCamp),
			BuildFixedSkillGroupId(normalizedZodiac, normalizedPrimaryElement));
	}

    public static string BuildFixedSkillGroupId(string? zodiac, string? primaryElement)
    {
        var normalizedZodiac = NormalizeRequiredChoice(zodiac, "生肖");
        var normalizedPrimaryElement = NormalizeRequiredChoice(primaryElement, "主元素");
        return $"{normalizedZodiac}_{normalizedPrimaryElement}";
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

    private static string NormalizeRequiredChoice(string? value, string fieldName)
    {
        if (string.IsNullOrWhiteSpace(value)
            || value.Trim().Equals("None", StringComparison.OrdinalIgnoreCase))
        {
            throw new ArgumentException($"{fieldName}不能为空或使用 None。", fieldName);
        }

        return value.Trim();
    }
}
