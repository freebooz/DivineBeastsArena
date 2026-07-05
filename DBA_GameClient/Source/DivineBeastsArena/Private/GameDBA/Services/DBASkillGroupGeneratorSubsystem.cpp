// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Services/DBASkillGroupGeneratorSubsystem.h"

#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "GameCore/Character/DBACharacterBuildTypes.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Misc/PackageName.h"

namespace
{
	FString GetEnumValueName(const UEnum* Enum, int64 Value)
	{
		return Enum ? Enum->GetNameStringByValue(Value) : FString();
	}

	bool IsEnumMaxName(const FString& Name)
	{
		return Name.Equals(TEXT("MAX"), ESearchCase::IgnoreCase) || Name.EndsWith(TEXT("_MAX"), ESearchCase::IgnoreCase);
	}

	FName MakeSkillGroupRowName(EDBAZodiac Zodiac, EDBAElement Element)
	{
		return DBACharacterBuild::MakeFixedSkillGroupId(Zodiac, Element);
	}

	bool HasValidSkillGroupIdentity(EDBAZodiac Zodiac, EDBAElement Element)
	{
		return Zodiac != EDBAZodiac::None
			&& Element != EDBAElement::None
			&& !MakeSkillGroupRowName(Zodiac, Element).IsNone();
	}

	bool DoesTablePackageExist(const TCHAR* PackagePath)
	{
		return PackagePath && FPackageName::DoesPackageExist(PackagePath);
	}

	void FillFallbackSkillGroup(EDBAZodiac Zodiac, EDBAElement Element, FDBAZodiacElementFixedSkillGroupRow& OutSkillGroup)
	{
		const FString ZodiacName = GetEnumValueName(StaticEnum<EDBAZodiac>(), static_cast<int64>(Zodiac));
		const FString ElementName = GetEnumValueName(StaticEnum<EDBAElement>(), static_cast<int64>(Element));
		const FName RowName = MakeSkillGroupRowName(Zodiac, Element);

		OutSkillGroup = FDBAZodiacElementFixedSkillGroupRow();
		OutSkillGroup.RowId = RowName;
		OutSkillGroup.ZodiacType = Zodiac;
		OutSkillGroup.ElementType = Element;
		OutSkillGroup.ElementPassiveSkillId = FName(*FString::Printf(TEXT("%s_%s_Passive"), *ZodiacName, *ElementName));
		OutSkillGroup.ElementSkill1Id = FName(*FString::Printf(TEXT("%s_%s_Skill01"), *ZodiacName, *ElementName));
		OutSkillGroup.ElementSkill2Id = FName(*FString::Printf(TEXT("%s_%s_Skill02"), *ZodiacName, *ElementName));
		OutSkillGroup.ElementSkill3Id = FName(*FString::Printf(TEXT("%s_%s_Skill03"), *ZodiacName, *ElementName));
		OutSkillGroup.ElementSkill4Id = FName(*FString::Printf(TEXT("%s_%s_Skill04"), *ZodiacName, *ElementName));
		OutSkillGroup.ZodiacUltimateSkillId = FName(*FString::Printf(TEXT("%s_Ultimate"), *ZodiacName));
		OutSkillGroup.ElementResonanceLevel = 4;
		OutSkillGroup.ResonanceElement = Element;
		OutSkillGroup.ResonanceControlTimeBonus = 1.0f;
		OutSkillGroup.ResonanceShieldBonus = 20.0f;
		OutSkillGroup.DisplayName = FText::FromString(FString::Printf(TEXT("%s %s \u6280\u80fd\u7ec4"), *ZodiacName, *ElementName));
		OutSkillGroup.Description = FText::FromString(TEXT("\u8fd0\u884c\u65f6\u515c\u5e95\u6280\u80fd\u7ec4"));
		OutSkillGroup.SanitizedName = RowName.ToString();
		OutSkillGroup.SanitizedAssetName = RowName.ToString();
		OutSkillGroup.bEnabled = true;
	}
}

UDBASkillGroupGeneratorSubsystem::UDBASkillGroupGeneratorSubsystem()
{
}

void UDBASkillGroupGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSkillGroupDataTable();
	LoadSkillGroupSummaryDataTable();
}

void UDBASkillGroupGeneratorSubsystem::Deinitialize()
{
	LoadedSkillGroupDataTable = nullptr;
	LoadedSkillGroupSummaryDataTable = nullptr;

	Super::Deinitialize();
}

void UDBASkillGroupGeneratorSubsystem::LoadSkillGroupDataTable()
{
	if (!DoesTablePackageExist(TEXT("/Game/DBA/Data/Tables/DT_FixedSkillGroups")))
	{
		UE_LOG(LogDBAData, Log, TEXT("[DBASkillGroupGeneratorSubsystem] \u56fa\u5b9a\u6280\u80fd\u7ec4\u6570\u636e\u8868\u4e0d\u5b58\u5728\uff0c\u542f\u7528\u8fd0\u884c\u65f6\u515c\u5e95\u6280\u80fd\u7ec4\u3002"));
		return;
	}

	if (SkillGroupDataTable.IsNull())
	{
		SkillGroupDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/DBA/Data/Tables/DT_FixedSkillGroups.DT_FixedSkillGroups")));
	}

	if (!LoadedSkillGroupDataTable)
	{
		if (UDataTable* ExistingTable = SkillGroupDataTable.Get())
		{
			LoadedSkillGroupDataTable = ExistingTable;
		}
		else
		{
			UAssetManager::GetStreamableManager().RequestAsyncLoad(
				SkillGroupDataTable.ToSoftObjectPath(),
				FStreamableDelegate::CreateWeakLambda(this, [this]()
				{
					LoadedSkillGroupDataTable = SkillGroupDataTable.Get();
				}),
				FStreamableManager::AsyncLoadHighPriority,
				true);
		}
	}
}

void UDBASkillGroupGeneratorSubsystem::LoadSkillGroupSummaryDataTable()
{
	if (!DoesTablePackageExist(TEXT("/Game/DBA/Data/Tables/DT_ZodiacSkillGroupSummaries")))
	{
		return;
	}

	if (SkillGroupSummaryDataTable.IsNull())
	{
		SkillGroupSummaryDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/DBA/Data/Tables/DT_ZodiacSkillGroupSummaries.DT_ZodiacSkillGroupSummaries")));
	}

	if (!LoadedSkillGroupSummaryDataTable)
	{
		if (UDataTable* ExistingTable = SkillGroupSummaryDataTable.Get())
		{
			LoadedSkillGroupSummaryDataTable = ExistingTable;
		}
		else
		{
			UAssetManager::GetStreamableManager().RequestAsyncLoad(
				SkillGroupSummaryDataTable.ToSoftObjectPath(),
				FStreamableDelegate::CreateWeakLambda(this, [this]()
				{
					LoadedSkillGroupSummaryDataTable = SkillGroupSummaryDataTable.Get();
				}),
				FStreamableManager::AsyncLoadHighPriority,
				true);
		}
	}
}

bool UDBASkillGroupGeneratorSubsystem::GetSkillGroup(EDBAZodiac Zodiac, EDBAElement Element, FDBAZodiacElementFixedSkillGroupRow& OutSkillGroup) const
{
	if (!HasValidSkillGroupIdentity(Zodiac, Element))
	{
		OutSkillGroup = FDBAZodiacElementFixedSkillGroupRow();
		return false;
	}

	const FName RowName = MakeSkillGroupRowName(Zodiac, Element);
	if (LoadedSkillGroupDataTable && FindSkillGroupByRowName(RowName, OutSkillGroup))
	{
		return true;
	}

	FillFallbackSkillGroup(Zodiac, Element, OutSkillGroup);
	return true;
}

bool UDBASkillGroupGeneratorSubsystem::GetSkillGroupSummary(EDBAZodiac Zodiac, FDBAZodiacHeroAbilitySetSummaryRow& OutSummary) const
{
	if (Zodiac == EDBAZodiac::None)
	{
		OutSummary = FDBAZodiacHeroAbilitySetSummaryRow();
		return false;
	}

	if (LoadedSkillGroupSummaryDataTable)
	{
		const FName RowName(*GetEnumValueName(StaticEnum<EDBAZodiac>(), static_cast<int64>(Zodiac)));
		const FDBAZodiacHeroAbilitySetSummaryRow* FoundRow = LoadedSkillGroupSummaryDataTable->FindRow<FDBAZodiacHeroAbilitySetSummaryRow>(RowName, TEXT(""));
		if (FoundRow)
		{
			OutSummary = *FoundRow;
			return true;
		}
	}

	OutSummary = FDBAZodiacHeroAbilitySetSummaryRow();
	OutSummary.Zodiac = Zodiac;
	OutSummary.MetalSkillGroupRowId = MakeSkillGroupRowName(Zodiac, EDBAElement::Gold);
	OutSummary.WoodSkillGroupRowId = MakeSkillGroupRowName(Zodiac, EDBAElement::Wood);
	OutSummary.WaterSkillGroupRowId = MakeSkillGroupRowName(Zodiac, EDBAElement::Water);
	OutSummary.FireSkillGroupRowId = MakeSkillGroupRowName(Zodiac, EDBAElement::Fire);
	OutSummary.EarthSkillGroupRowId = MakeSkillGroupRowName(Zodiac, EDBAElement::Earth);
	OutSummary.bAllSkillGroupsConfigured = true;
	OutSummary.ConfigurationCompleteness = 1.0f;
	return true;
}

TArray<EDBAZodiac> UDBASkillGroupGeneratorSubsystem::GetAllZodiacTypes() const
{
	TArray<EDBAZodiac> ZodiacTypes;

	const UEnum* Enum = StaticEnum<EDBAZodiac>();
	for (int32 Index = 0; Enum && Index < Enum->NumEnums(); ++Index)
	{
		const FString Name = Enum->GetNameStringByIndex(Index);
		const EDBAZodiac Zodiac = static_cast<EDBAZodiac>(Enum->GetValueByIndex(Index));
		if (Zodiac != EDBAZodiac::None && !IsEnumMaxName(Name))
		{
			ZodiacTypes.AddUnique(Zodiac);
		}
	}

	return ZodiacTypes;
}

TArray<EDBAElement> UDBASkillGroupGeneratorSubsystem::GetAllElementTypes() const
{
	TArray<EDBAElement> ElementTypes;

	const UEnum* Enum = StaticEnum<EDBAElement>();
	for (int32 Index = 0; Enum && Index < Enum->NumEnums(); ++Index)
	{
		const FString Name = Enum->GetNameStringByIndex(Index);
		const EDBAElement Element = static_cast<EDBAElement>(Enum->GetValueByIndex(Index));
		if (Element != EDBAElement::None && !IsEnumMaxName(Name))
		{
			ElementTypes.AddUnique(Element);
		}
	}

	return ElementTypes;
}

bool UDBASkillGroupGeneratorSubsystem::IsSkillGroupConfigured(EDBAZodiac Zodiac, EDBAElement Element) const
{
	FDBAZodiacElementFixedSkillGroupRow SkillGroup;
	return GetSkillGroup(Zodiac, Element, SkillGroup);
}

FName UDBASkillGroupGeneratorSubsystem::GetZodiacUltimateSkillId(EDBAZodiac Zodiac) const
{
	TArray<EDBAElement> Elements = GetAllElementTypes();
	if (Elements.Num() > 0)
	{
		FDBAZodiacElementFixedSkillGroupRow SkillGroup;
		if (GetSkillGroup(Zodiac, Elements[0], SkillGroup))
		{
			return SkillGroup.ZodiacUltimateSkillId;
		}
	}

	return NAME_None;
}

int32 UDBASkillGroupGeneratorSubsystem::CalculateResonanceLevel(const TArray<FName>& SkillIds, EDBAElement Element) const
{
	if (Element == EDBAElement::None)
	{
		return 0;
	}

	int32 SameElementSkillCount = 0;
	for (const FName& SkillId : SkillIds)
	{
		if (!SkillId.IsNone())
		{
			++SameElementSkillCount;
		}
	}

	if (SameElementSkillCount >= DBAConstants::ResonanceLevel4_SkillCount)
	{
		return 4;
	}
	if (SameElementSkillCount >= DBAConstants::ResonanceLevel3_SkillCount)
	{
		return 3;
	}
	if (SameElementSkillCount >= DBAConstants::ResonanceLevel2_SkillCount)
	{
		return 2;
	}
	if (SameElementSkillCount >= DBAConstants::ResonanceLevel1_SkillCount)
	{
		return 1;
	}

	return 0;
}

bool UDBASkillGroupGeneratorSubsystem::FindSkillGroupByRowName(const FName& RowName, FDBAZodiacElementFixedSkillGroupRow& OutSkillGroup) const
{
	if (!LoadedSkillGroupDataTable)
	{
		return false;
	}

	const FDBAZodiacElementFixedSkillGroupRow* FoundRow = LoadedSkillGroupDataTable->FindRow<FDBAZodiacElementFixedSkillGroupRow>(RowName, TEXT(""));
	if (!FoundRow)
	{
		return false;
	}

	if (!FoundRow->HasValidIdentity())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[DBASkillGroupGeneratorSubsystem] 固定技能组数据表行身份无效：%s"), *RowName.ToString());
		return false;
	}

	OutSkillGroup = *FoundRow;
	return true;
}
