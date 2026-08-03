/*
中文阅读说明：
- 所属应用：DBA_GameBackend 测试项目。
- 文件职责：验证 Dedicated Server Runtime player-joined 上报中的角色构建摘要不会绕过后端权威冻结值。
- 阅读重点：测试名称描述后端必须拒绝的篡改形态。
- 修改提示：调整 Zodiac / Element / FiveCamp 解耦规则时，请同步更新这里的固定技能组断言。
*/

using Game.Api.Services.Runtime;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Contracts.GameServer;
using System.Text.Json;

namespace Game.Api.Tests;

public class RuntimePlayerJoinBuildSummaryTests
{
    private static readonly Game.Application.Characters.ICharacterBuildPolicy BuildPolicy =
        TestCharacterBuildFactory.CreatePolicy();

    [Fact]
    public void ValidateBuildSummary_WhenFixedSkillGroupIsTampered_ReturnsFalse()
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid(),
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            FixedSkillGroupId = "Rat_Water"
        };

        var request = new RuntimePlayerJoinedRequest(
            Guid.NewGuid(),
            Guid.NewGuid(),
            "runtime-token",
            playerSession.PlayerId,
            "Blue",
            0,
            "player-session-token",
            "Rat",
            "Water",
            "East",
            "Rat_Fire");

        var result = RuntimePlayerJoinValidator.ValidateBuildSummary(playerSession, request, BuildPolicy);

        Assert.False(result.IsValid);
        Assert.Equal("Runtime player build summary does not match the frozen session build.", result.ErrorMessage);
    }

    [Fact]
    public void ValidateBuildSummary_WhenFiveCampChangesButSkillGroupMatches_ReturnsTrue()
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid(),
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            FixedSkillGroupId = "Rat_Water"
        };

        var request = new RuntimePlayerJoinedRequest(
            Guid.NewGuid(),
            Guid.NewGuid(),
            "runtime-token",
            playerSession.PlayerId,
            "Blue",
            0,
            "player-session-token",
            "Rat",
            "Water",
            "North",
            "Rat_Water");

        var result = RuntimePlayerJoinValidator.ValidateBuildSummary(playerSession, request, BuildPolicy);

        Assert.True(result.IsValid);
    }

    [Fact]
    public void ValidateBuildSummary_WhenFrozenChoicesArePaddedButValid_ReturnsTrue()
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid(),
            Zodiac = " Rat ",
            PrimaryElement = " Water ",
            FiveCamp = " East ",
            FixedSkillGroupId = " Rat_Water "
        };

        var request = new RuntimePlayerJoinedRequest(
            Guid.NewGuid(),
            Guid.NewGuid(),
            "runtime-token",
            playerSession.PlayerId,
            "Blue",
            0,
            "player-session-token",
            "Rat",
            "Water",
            "North",
            "Rat_Water");

        var result = RuntimePlayerJoinValidator.ValidateBuildSummary(playerSession, request, BuildPolicy);

        Assert.True(result.IsValid);
    }

    [Fact]
    public void ValidateBuildSummary_WhenFrozenFixedSkillGroupDoesNotMatchFrozenIdentity_ReturnsFalse()
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid(),
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            FixedSkillGroupId = "Tiger_Fire"
        };

        var request = new RuntimePlayerJoinedRequest(
            Guid.NewGuid(),
            Guid.NewGuid(),
            "runtime-token",
            playerSession.PlayerId,
            "Blue",
            0,
            "player-session-token",
            "Rat",
            "Water",
            "East",
            "Rat_Water");

        var result = RuntimePlayerJoinValidator.ValidateBuildSummary(playerSession, request, BuildPolicy);

        Assert.False(result.IsValid);
        Assert.Equal(TestCharacterBuildFactory.Options.Messages.FrozenBuildSummaryInvalid, result.ErrorMessage);
    }

    [Fact]
    public void ValidateBuildSummary_WhenNoFrozenBuildSummaryExists_AllowsLegacySession()
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid()
        };

        var request = new RuntimePlayerJoinedRequest(
            Guid.NewGuid(),
            Guid.NewGuid(),
            "runtime-token",
            playerSession.PlayerId,
            "Blue",
            0,
            "player-session-token",
            null,
            null,
            null,
            null);

        var result = RuntimePlayerJoinValidator.ValidateBuildSummary(playerSession, request, BuildPolicy);

        Assert.True(result.IsValid);
    }

    [Theory]
    [InlineData("Rat", "Water", null)]
    [InlineData("Rat", null, "Rat_Water")]
    [InlineData(null, "Water", "Rat_Water")]
    public void ValidateBuildSummary_WhenFrozenBuildSummaryIsPartial_ReturnsFalse(
        string? frozenZodiac,
        string? frozenElement,
        string? frozenFixedSkillGroupId)
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid(),
            Zodiac = frozenZodiac,
            PrimaryElement = frozenElement,
            FiveCamp = "East",
            FixedSkillGroupId = frozenFixedSkillGroupId
        };

        var request = new RuntimePlayerJoinedRequest(
            Guid.NewGuid(),
            Guid.NewGuid(),
            "runtime-token",
            playerSession.PlayerId,
            "Blue",
            0,
            "player-session-token",
            "Rat",
            "Water",
            "East",
            "Rat_Water");

        var result = RuntimePlayerJoinValidator.ValidateBuildSummary(playerSession, request, BuildPolicy);

        Assert.False(result.IsValid);
        Assert.Equal(TestCharacterBuildFactory.Options.Messages.FrozenBuildSummaryInvalid, result.ErrorMessage);
    }

    [Fact]
    public void ValidateBuildSummary_WhenFrozenBuildExistsButFixedSkillGroupIsMissing_ReturnsFalse()
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid(),
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            FixedSkillGroupId = "Rat_Water"
        };

        var request = new RuntimePlayerJoinedRequest(
            Guid.NewGuid(),
            Guid.NewGuid(),
            "runtime-token",
            playerSession.PlayerId,
            "Blue",
            0,
            "player-session-token",
            "Rat",
            "Water",
            "East",
            null);

        var result = RuntimePlayerJoinValidator.ValidateBuildSummary(playerSession, request, BuildPolicy);

        Assert.False(result.IsValid);
        Assert.Equal(TestCharacterBuildFactory.Options.Messages.BuildSummaryMissing, result.ErrorMessage);
    }

    [Theory]
    [InlineData("None", "Water", "Rat_Water")]
    [InlineData(" None ", "Water", "Rat_Water")]
    [InlineData("Rat", "None", "Rat_Water")]
    [InlineData("Rat", " None ", "Rat_Water")]
    [InlineData("Rat", "Water", "None")]
    [InlineData("Rat", "Water", " None ")]
    public void ValidateBuildSummary_WhenFrozenBuildExistsButRequiredChoiceIsNone_ReturnsFalse(
        string zodiac,
        string primaryElement,
        string fixedSkillGroupId)
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid(),
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            FixedSkillGroupId = "Rat_Water"
        };

        var request = new RuntimePlayerJoinedRequest(
            Guid.NewGuid(),
            Guid.NewGuid(),
            "runtime-token",
            playerSession.PlayerId,
            "Blue",
            0,
            "player-session-token",
            zodiac,
            primaryElement,
            "East",
            fixedSkillGroupId);

        var result = RuntimePlayerJoinValidator.ValidateBuildSummary(playerSession, request, BuildPolicy);

        Assert.False(result.IsValid);
        Assert.Equal(TestCharacterBuildFactory.Options.Messages.BuildSummaryMissing, result.ErrorMessage);
    }

    [Theory]
    [InlineData("None", "Water", "Rat_Water")]
    [InlineData(" None ", "Water", "Rat_Water")]
    [InlineData("Rat", "None", "Rat_Water")]
    [InlineData("Rat", " None ", "Rat_Water")]
    [InlineData("Rat", "Water", "None")]
    [InlineData("Rat", "Water", " None ")]
    public void ValidateBuildSummary_WhenFrozenBuildContainsNone_ReturnsFalse(
        string frozenZodiac,
        string frozenElement,
        string frozenFixedSkillGroupId)
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid(),
            Zodiac = frozenZodiac,
            PrimaryElement = frozenElement,
            FiveCamp = "East",
            FixedSkillGroupId = frozenFixedSkillGroupId
        };

        var request = new RuntimePlayerJoinedRequest(
            Guid.NewGuid(),
            Guid.NewGuid(),
            "runtime-token",
            playerSession.PlayerId,
            "Blue",
            0,
            "player-session-token",
            "Rat",
            "Water",
            "East",
            "Rat_Water");

        var result = RuntimePlayerJoinValidator.ValidateBuildSummary(playerSession, request, BuildPolicy);

        Assert.False(result.IsValid);
        Assert.Equal(TestCharacterBuildFactory.Options.Messages.FrozenBuildSummaryInvalid, result.ErrorMessage);
    }

    [Fact]
    public void BuildPlayerJoinedEventPayload_WhenFiveCampRequestDiffers_UsesFrozenBuildSummary()
    {
        var playerId = Guid.NewGuid();
        var playerSession = new PlayerSession
        {
            PlayerId = playerId,
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            FixedSkillGroupId = "Rat_Water"
        };

        var payloadJson = RuntimePlayerJoinValidator.BuildPlayerJoinedEventPayload(playerSession);

        using var payload = JsonDocument.Parse(payloadJson);
        var root = payload.RootElement;
        Assert.Equal(playerId, root.GetProperty("playerId").GetGuid());
        Assert.Equal("Rat", root.GetProperty("zodiac").GetString());
        Assert.Equal("Water", root.GetProperty("primaryElement").GetString());
        Assert.Equal("East", root.GetProperty("fiveCamp").GetString());
        Assert.Equal("Rat_Water", root.GetProperty("fixedSkillGroupId").GetString());
    }

    [Fact]
    public void BuildPlayerJoinedEventPayload_WhenTeamIsPadded_WritesNormalizedTeam()
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid(),
            Team = " blue ",
            Zodiac = "Rat",
            PrimaryElement = "Water",
            FiveCamp = "East",
            FixedSkillGroupId = "Rat_Water"
        };

        var payloadJson = RuntimePlayerJoinValidator.BuildPlayerJoinedEventPayload(playerSession);

        using var payload = JsonDocument.Parse(payloadJson);
        var root = payload.RootElement;
        Assert.Equal("blue", root.GetProperty("team").GetString());
    }

    [Fact]
    public void BuildPlayerJoinedEventPayload_WhenFrozenBuildSummaryIsPadded_WritesNormalizedSummary()
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid(),
            Zodiac = " Rat ",
            PrimaryElement = " Water ",
            FiveCamp = " East ",
            FixedSkillGroupId = " Rat_Water "
        };

        var payloadJson = RuntimePlayerJoinValidator.BuildPlayerJoinedEventPayload(playerSession);

        using var payload = JsonDocument.Parse(payloadJson);
        var root = payload.RootElement;
        Assert.Equal("Rat", root.GetProperty("zodiac").GetString());
        Assert.Equal("Water", root.GetProperty("primaryElement").GetString());
        Assert.Equal("East", root.GetProperty("fiveCamp").GetString());
        Assert.Equal("Rat_Water", root.GetProperty("fixedSkillGroupId").GetString());
    }

    [Fact]
    public void BuildPlayerJoinedEventPayload_WhenNoFrozenBuildSummaryExists_DoesNotInventDefaultSummary()
    {
        var playerSession = new PlayerSession
        {
            PlayerId = Guid.NewGuid()
        };

        var payloadJson = RuntimePlayerJoinValidator.BuildPlayerJoinedEventPayload(playerSession);

        using var payload = JsonDocument.Parse(payloadJson);
        var root = payload.RootElement;
        Assert.Equal(JsonValueKind.Null, root.GetProperty("zodiac").ValueKind);
        Assert.Equal(JsonValueKind.Null, root.GetProperty("primaryElement").ValueKind);
        Assert.Equal(JsonValueKind.Null, root.GetProperty("fiveCamp").ValueKind);
        Assert.Equal(JsonValueKind.Null, root.GetProperty("fixedSkillGroupId").ValueKind);
    }
}
