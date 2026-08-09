/*
中文阅读说明：
- 所属应用：DBA_GameBackend 共享配置层。
- 文件职责：承载角色创建规则和初始属性配置，避免在 API、应用层或 EF 仓储中硬编码业务数据。
- 修改提示：数值调整通过配置源完成；应用层只读取和校验，不复制默认值。
*/

namespace Game.Shared.Options;

public sealed class CharacterCreationOptions
{
    public const string Section = "CharacterCreation";

    public string GeneratedNamePrefix { get; init; } = string.Empty;
    public int GeneratedNameSuffixLength { get; init; }
    public string NamePattern { get; init; } = string.Empty;
    public string DefaultZodiac { get; init; } = string.Empty;
    public string DefaultPrimaryElement { get; init; } = string.Empty;
    public string DefaultFiveCamp { get; init; } = string.Empty;
    /** 旧 /api/account/characters 兼容入口使用的默认区服；新 v1 接口必须显式传 ServerId。 */
    public string DefaultServerId { get; init; } = string.Empty;
    public int InitialLevel { get; init; }
    public int MaxSlotsPerServer { get; init; }
    public string RulesVersion { get; init; } = string.Empty;
    public List<string> AllowedZodiacTypes { get; init; } = [];
    public List<string> AllowedElementTypes { get; init; } = [];
    public List<string> AllowedFiveCampTypes { get; init; } = [];
    public List<string> ReservedNames { get; init; } = [];
    public Dictionary<string, CharacterAppearanceRuleOptions> AppearanceRules { get; init; } = new(StringComparer.OrdinalIgnoreCase);
    public CharacterCoreAttributesOptions CoreAttributes { get; init; } = new();
    public CharacterErrorMessagesOptions Messages { get; init; } = new();
}

public sealed class CharacterAppearanceRuleOptions
{
    public List<string> AllowedOptionIds { get; init; } = [];
}

public sealed class CharacterCoreAttributesOptions
{
    public float MaxHealth { get; init; }
    public float AttackPower { get; init; }
    public float Defense { get; init; }
    public float MoveSpeed { get; init; }
    public float MaxEnergy { get; init; }
    public float EnergyRegen { get; init; }
    public float CriticalRate { get; init; }
    public float CriticalMultiplier { get; init; }
}

public sealed class CharacterErrorMessagesOptions
{
    public string InvalidPlayer { get; init; } = string.Empty;
    public string InvalidName { get; init; } = string.Empty;
    public string DuplicateName { get; init; } = string.Empty;
    public string InvalidCharacterId { get; init; } = string.Empty;
    public string CharacterNotFound { get; init; } = string.Empty;
    public string BuildSummaryMismatch { get; init; } = string.Empty;
    public string BuildSummaryMissing { get; init; } = string.Empty;
    public string FrozenBuildSummaryInvalid { get; init; } = string.Empty;
}
