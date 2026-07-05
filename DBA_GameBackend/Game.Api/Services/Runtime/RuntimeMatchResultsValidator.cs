/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：验证 Runtime 对局结果上报并构造结算服务 payload，避免 endpoint 内联承载业务边界。
- 阅读重点：玩家列表必须来自当前会话，结果上报必须可幂等，结算字段映射必须稳定。
- 修改提示：新增结算字段或 Runtime 结果规则时，请同步更新 RuntimeMatchResultsValidatorTests。
*/

using Game.Shared.Contracts.GameServer;
using SettlementSubmitMatchResultRequest = Game.Shared.Contracts.Settlement.SubmitMatchResultRequest;
using SettlementMatchPlayerResultDto = Game.Shared.Contracts.Settlement.MatchPlayerResultDto;
using System.Text.Json;

namespace Game.Api.Services.Runtime;

public sealed record RuntimeMatchResultsValidationResult(
    bool IsValid,
    string? ErrorMessage,
    SettlementSubmitMatchResultRequest? Payload);

public static class RuntimeMatchResultsValidator
{
    public const string MissingIdempotencyKeyMessage = "Match result idempotency key is required.";
    public const string MissingPlayersMessage = "Match result must contain at least one player.";
    public const string DuplicatePlayersMessage = "Match result contains duplicate players.";
    public const string UnknownPlayersMessage = "Match result contains players not in session";
    public const string MissingSessionPlayersMessage = "Match result is missing players from session.";
    public const string MissingPlayerTeamMessage = "Match result contains players without a team.";
    public const string TeamMismatchMessage = "Match result contains a player team that does not match the session.";
    public const string InvalidPlayerResultMessage = "Match result contains an invalid player result.";
    public const string InvalidPlayerNumericValueMessage = "Match result contains an invalid player numeric value.";

    public static RuntimeMatchResultsValidationResult ValidateAndBuildPayload(
        RuntimeMatchResultsRequest request,
        IReadOnlyDictionary<Guid, string?> sessionPlayerTeams)
    {
        if (string.IsNullOrWhiteSpace(request.IdempotencyKey))
        {
            return Invalid(MissingIdempotencyKeyMessage);
        }

        if (request.Players.Count == 0)
        {
            return Invalid(MissingPlayersMessage);
        }

        var reportedPlayerIds = request.Players.Select(x => x.PlayerId).ToList();
        if (reportedPlayerIds.Distinct().Count() != reportedPlayerIds.Count)
        {
            return Invalid(DuplicatePlayersMessage);
        }

        if (reportedPlayerIds.Any(x => !sessionPlayerTeams.ContainsKey(x)))
        {
            return Invalid(UnknownPlayersMessage);
        }

        if (sessionPlayerTeams.Keys.Any(x => !reportedPlayerIds.Contains(x)))
        {
            return Invalid(MissingSessionPlayersMessage);
        }

        foreach (var player in request.Players)
        {
            var reportedTeam = NormalizeTeam(player.Team);
            if (reportedTeam.Length == 0)
            {
                return Invalid(MissingPlayerTeamMessage);
            }

            var expectedTeam = NormalizeTeam(sessionPlayerTeams[player.PlayerId]);
            if (expectedTeam.Length == 0 || !string.Equals(reportedTeam, expectedTeam, StringComparison.Ordinal))
            {
                return Invalid(TeamMismatchMessage);
            }

            if (!IsValidPlayerResult(NormalizePlayerResult(player.Result)))
            {
                return Invalid(InvalidPlayerResultMessage);
            }

            if (!HasValidNonNegativePlayerValues(player))
            {
                return Invalid(InvalidPlayerNumericValueMessage);
            }
        }

        var payload = new SettlementSubmitMatchResultRequest(
            request.SessionId,
            request.IdempotencyKey.Trim(),
            string.IsNullOrWhiteSpace(request.ResultJson) ? JsonSerializer.Serialize(request) : request.ResultJson,
            request.Players.Select(x => new SettlementMatchPlayerResultDto(
                x.PlayerId,
                NormalizeTeam(sessionPlayerTeams[x.PlayerId]),
                NormalizePlayerResult(x.Result),
                x.Kills,
                x.Deaths,
                x.Assists,
                x.Score,
                x.ExpDelta,
                x.Rewards)).ToList());

        return new RuntimeMatchResultsValidationResult(true, null, payload);
    }

    private static RuntimeMatchResultsValidationResult Invalid(string message)
    {
        return new RuntimeMatchResultsValidationResult(false, message, null);
    }

    private static string NormalizeTeam(string? team)
    {
        return string.IsNullOrWhiteSpace(team)
            ? string.Empty
            : team.Trim().ToLowerInvariant();
    }

    private static string NormalizePlayerResult(string? result)
    {
        return string.IsNullOrWhiteSpace(result)
            ? string.Empty
            : result.Trim().ToLowerInvariant();
    }

    private static bool IsValidPlayerResult(string result)
    {
        return result is "win" or "loss" or "draw";
    }

    private static bool HasValidNonNegativePlayerValues(RuntimePlayerResultDto player)
    {
        return player.Kills >= 0 &&
               player.Deaths >= 0 &&
               player.Assists >= 0 &&
               player.Score >= 0 &&
               player.ExpDelta >= 0 &&
               player.Rewards.All(x => IsNonNegativeRewardQuantity(x.Value));
    }

    private static bool IsNonNegativeRewardQuantity(object value)
    {
        return value switch
        {
            int quantity => quantity >= 0,
            long quantity => quantity >= 0,
            decimal quantity => quantity >= 0,
            double quantity => quantity >= 0,
            JsonElement { ValueKind: JsonValueKind.Number } quantity =>
                quantity.TryGetInt64(out var longQuantity)
                    ? longQuantity >= 0
                    : quantity.TryGetDouble(out var doubleQuantity) && doubleQuantity >= 0,
            _ => true
        };
    }
}
