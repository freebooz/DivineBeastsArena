/*
中文阅读说明：
- 所属应用：DBA_GameBackend Runtime 服务。
- 文件职责：校验 Dedicated Server 上报的玩家入服构建摘要与后端冻结摘要一致。
- 阅读重点：FiveCamp 只做表现包，允许变化；FixedSkillGroupId 必须由 Zodiac + Element 决定。
- 修改提示：如扩展冻结摘要字段，先补测试再修改这里。
*/

using Game.Application.Characters;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Contracts.GameServer;
using System.Text.Json;

namespace Game.Api.Services.Runtime;

public readonly record struct RuntimePlayerJoinValidationResult(bool IsValid, string? ErrorMessage = null);

public static class RuntimePlayerJoinValidator
{
    public static RuntimePlayerJoinValidationResult ValidateBuildSummary(
        PlayerSession playerSession,
        RuntimePlayerJoinedRequest request,
        ICharacterBuildPolicy characterBuildPolicy)
    {
        var result = characterBuildPolicy.ValidateRuntimeJoin(
            new CharacterBuildSnapshot(
                playerSession.Zodiac,
                playerSession.PrimaryElement,
                playerSession.FiveCamp,
                playerSession.FixedSkillGroupId),
            new CharacterBuildSnapshot(
                request.Zodiac,
                request.PrimaryElement,
                request.FiveCamp,
                request.FixedSkillGroupId));
        return new RuntimePlayerJoinValidationResult(result.IsValid, result.ErrorMessage);
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

}
