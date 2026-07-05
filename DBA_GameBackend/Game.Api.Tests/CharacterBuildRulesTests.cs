/*
中文阅读说明：
- 所属应用：DBA_GameBackend 测试项目。
- 文件职责：验证后端共享角色构建规则，避免 Account、Session、Runtime 各自复制 FixedSkillGroupId 生成逻辑。
- 阅读重点：FixedSkillGroupId 只能由 Zodiac + PrimaryElement 决定；FiveCamp 是表现维度。
- 修改提示：若 Zodiac / Element 默认值或固定技能组命名规则变化，先改这里再同步实现。
*/

using Game.Shared.Contracts.Character;

namespace Game.Api.Tests;

public class CharacterBuildRulesTests
{
    [Fact]
    public void BuildFixedSkillGroupId_WhenZodiacAndElementAreProvided_UsesZodiacAndElement()
    {
        var fixedSkillGroupId = CharacterBuildRules.BuildFixedSkillGroupId("Tiger", "Fire");

        Assert.Equal("Tiger_Fire", fixedSkillGroupId);
    }

    [Theory]
    [InlineData(null, null, "Rat_Water")]
    [InlineData("", "", "Rat_Water")]
    [InlineData("None", "None", "Rat_Water")]
    [InlineData(" None ", " None ", "Rat_Water")]
    [InlineData(" Rat ", " Water ", "Rat_Water")]
    public void BuildFixedSkillGroupId_WhenValuesAreMissingOrPadded_NormalizesChoices(
        string? zodiac,
        string? element,
        string expected)
    {
        var fixedSkillGroupId = CharacterBuildRules.BuildFixedSkillGroupId(zodiac, element);

        Assert.Equal(expected, fixedSkillGroupId);
    }

    [Fact]
    public void BuildFixedSkillGroupId_WhenFiveCampChanges_DoesNotChangeSkillGroup()
    {
        var east = CharacterBuildRules.BuildFixedSkillGroupId("Rat", "Water");
        var north = CharacterBuildRules.BuildFixedSkillGroupId("Rat", "Water");

        Assert.Equal(east, north);
    }

    [Theory]
    [InlineData(null, null, null, "Rat", "Water", "East", "Rat_Water")]
    [InlineData(" None ", " None ", " None ", "Rat", "Water", "East", "Rat_Water")]
    [InlineData(" Tiger ", " Fire ", " South ", "Tiger", "Fire", "South", "Tiger_Fire")]
    public void BuildSummary_WhenValuesAreMissingOrPadded_NormalizesAllFields(
        string? zodiac,
        string? primaryElement,
        string? fiveCamp,
        string expectedZodiac,
        string expectedPrimaryElement,
        string expectedFiveCamp,
        string expectedFixedSkillGroupId)
    {
        var buildSummary = CharacterBuildRules.BuildSummary(zodiac, primaryElement, fiveCamp);

        Assert.Equal(expectedZodiac, buildSummary.Zodiac);
        Assert.Equal(expectedPrimaryElement, buildSummary.PrimaryElement);
        Assert.Equal(expectedFiveCamp, buildSummary.FiveCamp);
        Assert.Equal(expectedFixedSkillGroupId, buildSummary.FixedSkillGroupId);
    }
}
