// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Gameplay/Loadout/SkillGroups/DBASkillGroupGeneratorSubsystem.h"
#include "GameDBA/Gameplay/Loadout/SkillGroups/DBASkillGroupDeveloperSettings.h"

#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "GameCore/Types/DBACharacterBuildTypes.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"

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

	bool HasValidSkillGroupIdentity(EDBAZodiac Zodiac, EDBAElement Element)
	{
		return Zodiac != EDBAZodiac::None
			&& Element != EDBAElement::None;
	}

	FName FindSkillGroupRowName(const UDataTable* DataTable, EDBAZodiac Zodiac, EDBAElement Element)
	{
		if (!DataTable)
		{
			return NAME_None;
		}

		TArray<FDBAZodiacElementFixedSkillGroupRow*> SkillGroups;
		DataTable->GetAllRows(TEXT("固定技能组查询"), SkillGroups);
		for (const FDBAZodiacElementFixedSkillGroupRow* SkillGroup : SkillGroups)
		{
			if (SkillGroup && SkillGroup->ZodiacType == Zodiac && SkillGroup->ElementType == Element)
			{
				return SkillGroup->RowId;
			}
		}

		return NAME_None;
	}

	FName GetStableEnumId(const UEnum* Enum, const int64 Value)
	{
		return Enum ? FName(*Enum->GetNameStringByValue(Value)) : NAME_None;
	}

}

void UDBASkillGroupGeneratorSubsystem::OnSubsystemInitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Initialize，此处仅执行派生类初始化
	LoadSkillGroupDataTable();
	LoadSkillGroupSummaryDataTable();
}

void UDBASkillGroupGeneratorSubsystem::OnSubsystemDeinitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Deinitialize，此处仅清理派生类状态
	LoadedSkillGroupDataTable = nullptr;
	LoadedSkillGroupSummaryDataTable = nullptr;
	OnSkillGroupDataReady.Clear();
}

void UDBASkillGroupGeneratorSubsystem::LoadSkillGroupDataTable()
{
	const UDBASkillGroupDeveloperSettings* Settings = GetDefault<UDBASkillGroupDeveloperSettings>();
	if (!Settings || Settings->DefaultSkillGroupDataTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[DBASkillGroupGeneratorSubsystem] 未配置固定技能组主表，无法异步加载数据表。"));
		return;
	}

	SkillGroupDataTable = Settings->DefaultSkillGroupDataTable;

	if (!LoadedSkillGroupDataTable)
	{
		if (UDataTable* ExistingTable = SkillGroupDataTable.Get())
		{
			LoadedSkillGroupDataTable = ExistingTable;
			OnSkillGroupDataReady.Broadcast();
		}
		else
		{
			UAssetManager::GetStreamableManager().RequestAsyncLoad(
				SkillGroupDataTable.ToSoftObjectPath(),
				FStreamableDelegate::CreateWeakLambda(this, [this]()
				{
					LoadedSkillGroupDataTable = SkillGroupDataTable.Get();
					if (LoadedSkillGroupDataTable)
					{
						OnSkillGroupDataReady.Broadcast();
					}
				}),
				FStreamableManager::AsyncLoadHighPriority,
				true);
		}
	}
}

void UDBASkillGroupGeneratorSubsystem::LoadSkillGroupSummaryDataTable()
{
	const UDBASkillGroupDeveloperSettings* Settings = GetDefault<UDBASkillGroupDeveloperSettings>();
	if (!Settings || Settings->DefaultSkillGroupSummaryDataTable.IsNull())
	{
		UE_LOG(LogDBAData, Log, TEXT("[DBASkillGroupGeneratorSubsystem] 未配置技能组摘要表，将由固定技能组主表生成摘要。"));
		return;
	}

	SkillGroupSummaryDataTable = Settings->DefaultSkillGroupSummaryDataTable;

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

	const FName RowName = FindSkillGroupRowName(LoadedSkillGroupDataTable, Zodiac, Element);
	if (LoadedSkillGroupDataTable && FindSkillGroupByRowName(RowName, OutSkillGroup))
	{
		return true;
	}

	OutSkillGroup = FDBAZodiacElementFixedSkillGroupRow();
	return false;
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

	if (!LoadedSkillGroupDataTable)
	{
		OutSummary = FDBAZodiacHeroAbilitySetSummaryRow();
		return false;
	}

	OutSummary = FDBAZodiacHeroAbilitySetSummaryRow();
	OutSummary.Zodiac = Zodiac;
	OutSummary.MetalSkillGroupRowId = FindSkillGroupRowName(LoadedSkillGroupDataTable, Zodiac, EDBAElement::Gold);
	OutSummary.WoodSkillGroupRowId = FindSkillGroupRowName(LoadedSkillGroupDataTable, Zodiac, EDBAElement::Wood);
	OutSummary.WaterSkillGroupRowId = FindSkillGroupRowName(LoadedSkillGroupDataTable, Zodiac, EDBAElement::Water);
	OutSummary.FireSkillGroupRowId = FindSkillGroupRowName(LoadedSkillGroupDataTable, Zodiac, EDBAElement::Fire);
	OutSummary.EarthSkillGroupRowId = FindSkillGroupRowName(LoadedSkillGroupDataTable, Zodiac, EDBAElement::Earth);
	const TArray<FName> RequiredRowIds = {
		OutSummary.MetalSkillGroupRowId,
		OutSummary.WoodSkillGroupRowId,
		OutSummary.WaterSkillGroupRowId,
		OutSummary.FireSkillGroupRowId,
		OutSummary.EarthSkillGroupRowId
	};

	int32 ValidRowCount = 0;
	for (const FName& RowId : RequiredRowIds)
	{
		FDBAZodiacElementFixedSkillGroupRow SkillGroup;
		if (FindSkillGroupByRowName(RowId, SkillGroup))
		{
			++ValidRowCount;
		}
	}

	OutSummary.bAllSkillGroupsConfigured = ValidRowCount == RequiredRowIds.Num();
	OutSummary.ConfigurationCompleteness = RequiredRowIds.Num() > 0
		? static_cast<float>(ValidRowCount) / static_cast<float>(RequiredRowIds.Num())
		: 0.0f;
	return OutSummary.bAllSkillGroupsConfigured;
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

bool UDBASkillGroupGeneratorSubsystem::IsBuildIdentityConfigured(const FDBACharacterBuildSummary& BuildIdentity) const
{
	if (!BuildIdentity.IsValid() || !LoadedSkillGroupDataTable)
	{
		return false;
	}

	TArray<FDBAZodiacElementFixedSkillGroupRow*> SkillGroups;
	LoadedSkillGroupDataTable->GetAllRows(TEXT("构筑身份校验"), SkillGroups);
	for (const FDBAZodiacElementFixedSkillGroupRow* SkillGroup : SkillGroups)
	{
		if (!SkillGroup || !SkillGroup->bEnabled || SkillGroup->bIsInDevelopment)
		{
			continue;
		}

		const FName ZodiacId = GetStableEnumId(StaticEnum<EDBAZodiac>(), static_cast<int64>(SkillGroup->ZodiacType));
		const FName ElementId = GetStableEnumId(StaticEnum<EDBAElement>(), static_cast<int64>(SkillGroup->ElementType));
		if (BuildIdentity.ZodiacId == ZodiacId
			&& BuildIdentity.PrimaryElementId == ElementId
			&& BuildIdentity.FixedSkillGroupId == SkillGroup->RowId)
		{
			return true;
		}
	}

	return false;
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
