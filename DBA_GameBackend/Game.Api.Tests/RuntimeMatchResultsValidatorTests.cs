/*
中文阅读说明：
- 所属应用：DBA_GameBackend 测试项目。
- 文件职责：验证 Runtime 对局结果上报进入 SettlementService 前的准入和字段映射。
- 阅读重点：Dedicated Server 只能结算本局玩家，不能空提交或重复玩家，payload 映射必须保持稳定。
- 修改提示：调整 Runtime 结算 DTO 或奖励字段时，请同步更新这些契约断言。
*/

using Game.Api.Services.Runtime;
using Game.Shared.Contracts.GameServer;
using System.Text.Json;

namespace Game.Api.Tests;

public class RuntimeMatchResultsValidatorTests
{
    [Fact]
    public void ValidateAndBuildPayload_WithKnownPlayers_MapsSettlementPayload()
    {
        var sessionId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        var request = CreateRequest(sessionId, playerId);

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(playerId, "blue"));

        Assert.True(result.IsValid);
        Assert.Null(result.ErrorMessage);
        Assert.NotNull(result.Payload);
        Assert.Equal(sessionId, result.Payload!.SessionId);
        Assert.Equal("match-result-001", result.Payload.IdempotencyKey);
        Assert.Equal("""{"winner":"blue"}""", result.Payload.ResultJson);
        var player = Assert.Single(result.Payload.Players);
        Assert.Equal(playerId, player.PlayerId);
        Assert.Equal("blue", player.Team);
        Assert.Equal("win", player.Result);
        Assert.Equal(4, player.Kills);
        Assert.Equal(1, player.Deaths);
        Assert.Equal(2, player.Assists);
        Assert.Equal(900, player.Score);
        Assert.Equal(1200, player.ExpDelta);
        Assert.Equal(25, Convert.ToInt32(player.Rewards["coin"]));
        Assert.Equal(3, Convert.ToInt32(player.Rewards["honor"]));
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenResultJsonIsBlank_UsesSerializedRuntimeRequest()
    {
        var sessionId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        var request = CreateRequest(sessionId, playerId) with { ResultJson = " " };

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(playerId, "blue"));

        Assert.True(result.IsValid);
        Assert.NotNull(result.Payload);
        using var payload = JsonDocument.Parse(result.Payload!.ResultJson);
        Assert.Equal(sessionId, payload.RootElement.GetProperty("SessionId").GetGuid());
        Assert.Equal("match-result-001", payload.RootElement.GetProperty("IdempotencyKey").GetString());
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenIdempotencyKeyIsBlank_ReturnsInvalid()
    {
        var playerId = Guid.NewGuid();
        var request = CreateRequest(Guid.NewGuid(), playerId) with { IdempotencyKey = " " };

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(playerId, "blue"));

        Assert.False(result.IsValid);
        Assert.Equal(RuntimeMatchResultsValidator.MissingIdempotencyKeyMessage, result.ErrorMessage);
        Assert.Null(result.Payload);
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenPlayersAreEmpty_ReturnsInvalid()
    {
        var request = CreateRequest(Guid.NewGuid(), Guid.NewGuid()) with
        {
            Players = Array.Empty<RuntimePlayerResultDto>()
        };

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            new Dictionary<Guid, string?>());

        Assert.False(result.IsValid);
        Assert.Equal(RuntimeMatchResultsValidator.MissingPlayersMessage, result.ErrorMessage);
        Assert.Null(result.Payload);
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenPlayerIsNotInSession_ReturnsInvalid()
    {
        var request = CreateRequest(Guid.NewGuid(), Guid.NewGuid());

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(Guid.NewGuid(), "blue"));

        Assert.False(result.IsValid);
        Assert.Equal(RuntimeMatchResultsValidator.UnknownPlayersMessage, result.ErrorMessage);
        Assert.Null(result.Payload);
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenSessionPlayerIsMissing_ReturnsInvalid()
    {
        var playerId = Guid.NewGuid();
        var missingPlayerId = Guid.NewGuid();
        var request = CreateRequest(Guid.NewGuid(), playerId);

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            new Dictionary<Guid, string?>
            {
                [playerId] = "blue",
                [missingPlayerId] = "red"
            });

        Assert.False(result.IsValid);
        Assert.Equal(RuntimeMatchResultsValidator.MissingSessionPlayersMessage, result.ErrorMessage);
        Assert.Null(result.Payload);
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenPlayerIsDuplicated_ReturnsInvalid()
    {
        var sessionId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        var playerResult = CreatePlayerResult(playerId);
        var request = CreateRequest(sessionId, playerId) with
        {
            Players = new[] { playerResult, playerResult }
        };

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(playerId, "blue"));

        Assert.False(result.IsValid);
        Assert.Equal(RuntimeMatchResultsValidator.DuplicatePlayersMessage, result.ErrorMessage);
        Assert.Null(result.Payload);
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenPlayerTeamIsMissing_ReturnsInvalid()
    {
        var playerId = Guid.NewGuid();
        var request = CreateRequest(Guid.NewGuid(), playerId) with
        {
            Players = new[] { CreatePlayerResult(playerId) with { Team = " " } }
        };

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(playerId, "blue"));

        Assert.False(result.IsValid);
        Assert.Equal(RuntimeMatchResultsValidator.MissingPlayerTeamMessage, result.ErrorMessage);
        Assert.Null(result.Payload);
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenPlayerTeamDoesNotMatchSession_ReturnsInvalid()
    {
        var playerId = Guid.NewGuid();
        var request = CreateRequest(Guid.NewGuid(), playerId) with
        {
            Players = new[] { CreatePlayerResult(playerId) with { Team = "red" } }
        };

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(playerId, "blue"));

        Assert.False(result.IsValid);
        Assert.Equal(RuntimeMatchResultsValidator.TeamMismatchMessage, result.ErrorMessage);
        Assert.Null(result.Payload);
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenPlayerResultIsInvalid_ReturnsInvalid()
    {
        var playerId = Guid.NewGuid();
        var request = CreateRequest(Guid.NewGuid(), playerId) with
        {
            Players = new[] { CreatePlayerResult(playerId) with { Result = "eliminated" } }
        };

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(playerId, "blue"));

        Assert.False(result.IsValid);
        Assert.Equal(RuntimeMatchResultsValidator.InvalidPlayerResultMessage, result.ErrorMessage);
        Assert.Null(result.Payload);
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenPlayerStatsContainNegativeValue_ReturnsInvalid()
    {
        var playerId = Guid.NewGuid();
        var request = CreateRequest(Guid.NewGuid(), playerId) with
        {
            Players = new[] { CreatePlayerResult(playerId) with { ExpDelta = -1 } }
        };

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(playerId, "blue"));

        Assert.False(result.IsValid);
        Assert.Equal(RuntimeMatchResultsValidator.InvalidPlayerNumericValueMessage, result.ErrorMessage);
        Assert.Null(result.Payload);
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenRewardQuantityIsNegative_ReturnsInvalid()
    {
        var playerId = Guid.NewGuid();
        var request = CreateRequest(Guid.NewGuid(), playerId) with
        {
            Players = new[]
            {
                CreatePlayerResult(playerId) with
                {
                    Rewards = new Dictionary<string, object> { ["coin"] = -5 }
                }
            }
        };

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(playerId, "blue"));

        Assert.False(result.IsValid);
        Assert.Equal(RuntimeMatchResultsValidator.InvalidPlayerNumericValueMessage, result.ErrorMessage);
        Assert.Null(result.Payload);
    }

    [Fact]
    public void ValidateAndBuildPayload_WhenReportedTeamHasDifferentCasing_UsesSessionTeamInPayload()
    {
        var playerId = Guid.NewGuid();
        var request = CreateRequest(Guid.NewGuid(), playerId) with
        {
            Players = new[] { CreatePlayerResult(playerId) with { Team = " BLUE " } }
        };

        var result = RuntimeMatchResultsValidator.ValidateAndBuildPayload(
            request,
            CreateSessionTeams(playerId, "blue"));

        Assert.True(result.IsValid);
        Assert.NotNull(result.Payload);
        var player = Assert.Single(result.Payload!.Players);
        Assert.Equal("blue", player.Team);
    }

    private static RuntimeMatchResultsRequest CreateRequest(Guid sessionId, Guid playerId)
    {
        return new RuntimeMatchResultsRequest(
            Guid.NewGuid(),
            sessionId,
            "runtime-token-001",
            "match-result-001",
            """{"winner":"blue"}""",
            new[] { CreatePlayerResult(playerId) });
    }

    private static RuntimePlayerResultDto CreatePlayerResult(Guid playerId)
    {
        return new RuntimePlayerResultDto(
            playerId,
            "blue",
            "win",
            Kills: 4,
            Deaths: 1,
            Assists: 2,
            Score: 900,
            ExpDelta: 1200,
            Rewards: new Dictionary<string, object>
            {
                ["coin"] = 25,
                ["honor"] = 3
            });
    }

    private static IReadOnlyDictionary<Guid, string?> CreateSessionTeams(Guid playerId, string? team)
    {
        return new Dictionary<Guid, string?> { [playerId] = team };
    }
}
