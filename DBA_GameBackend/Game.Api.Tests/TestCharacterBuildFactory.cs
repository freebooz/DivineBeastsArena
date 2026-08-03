/*
中文阅读说明：
- 所属应用：DBA_GameBackend 测试夹具。
- 文件职责：为手工构造的应用服务提供角色配置，测试夹具允许使用固定样例数据。
*/

using Game.Application.Characters;
using Game.Application.Sessions;
using Game.Infrastructure.Security;
using Game.Shared.Options;

namespace Game.Api.Tests;

internal static class TestCharacterBuildFactory
{
    internal static SessionAdmissionOptions SessionAdmissionOptions { get; } = new()
    {
        TokenByteLength = 32,
        ConnectionTokenLifetimeMinutes = 10,
        ReconnectTokenLifetimeMinutes = 60,
        ProvisionalTokenLifetimeMinutes = 120,
        MatchMapId = "LobbyMap",
        MatchMaxPlayers = 2
    };

    internal static CharacterCreationOptions Options { get; } = new()
    {
        GeneratedNamePrefix = "测试角色_",
        GeneratedNameSuffixLength = 8,
        NamePattern = "^[\\u4e00-\\u9fa5A-Za-z0-9_]{2,16}$",
        DefaultZodiac = "Rat",
        DefaultPrimaryElement = "Water",
        DefaultFiveCamp = "East",
        InitialLevel = 1,
        CoreAttributes = new CharacterCoreAttributesOptions
        {
            MaxHealth = 1800,
            AttackPower = 100,
            Defense = 40,
            MoveSpeed = 380,
            MaxEnergy = 100,
            EnergyRegen = 10,
            CriticalRate = 0.05f,
            CriticalMultiplier = 2.0f
        },
        Messages = new CharacterErrorMessagesOptions
        {
            InvalidPlayer = "玩家身份无效。",
            InvalidName = "角色名称无效。",
            DuplicateName = "角色名称已经存在。",
            InvalidCharacterId = "角色标识无效。",
            CharacterNotFound = "未找到所属角色。",
            BuildSummaryMismatch = "玩家入服构筑与会话冻结构筑不一致。",
            BuildSummaryMissing = "会话已有冻结构筑，但入服请求缺少构筑摘要。",
            FrozenBuildSummaryInvalid = "会话冻结构筑无效。"
        }
    };

    internal static ICharacterBuildPolicy CreatePolicy()
    {
        return new CharacterBuildPolicy(Options);
    }

    internal static ISessionCredentialIssuer CreateCredentialIssuer(TimeProvider? timeProvider = null)
    {
        return new CryptographicSessionCredentialIssuer(
            SessionAdmissionOptions,
            timeProvider ?? TimeProvider.System);
    }
}
