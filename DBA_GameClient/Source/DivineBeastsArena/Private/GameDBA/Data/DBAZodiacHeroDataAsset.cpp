// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Data/DBAZodiacHeroDataAsset.h"
#include "Engine/DataTable.h"
#include "GameCore/Character/DBACharacterBuildTypes.h"
#include "GameDBA/Core/DBAConstants.h"

UDBAZodiacHeroDataAsset::UDBAZodiacHeroDataAsset()
{
}

FName UDBAZodiacHeroDataAsset::BuildFixedSkillGroupRowName(EDBAZodiac Zodiac, EDBAElement Element)
{
	return DBACharacterBuild::MakeFixedSkillGroupId(Zodiac, Element);
}

bool UDBAZodiacHeroDataAsset::GetZodiacHeroDisplayData(EDBAZodiac Zodiac, FDBAZodiacHeroDisplayRow& OutRow) const
{
	if (Zodiac == EDBAZodiac::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(ZodiacHeroDisplayTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildZodiacRowName(Zodiac);
	return FindDataRow(DataTable, RowName, OutRow);
}

bool UDBAZodiacHeroDataAsset::GetZodiacHeroConfigData(EDBAZodiac Zodiac, FDBAZodiacHeroConfigRow& OutRow) const
{
	if (Zodiac == EDBAZodiac::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(ZodiacHeroConfigTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildZodiacRowName(Zodiac);
	return FindDataRow(DataTable, RowName, OutRow);
}

bool UDBAZodiacHeroDataAsset::GetFixedSkillGroupData(EDBAZodiac Zodiac, EDBAElement Element, FDBAZodiacElementFixedSkillGroupRow& OutRow) const
{
	if (Zodiac == EDBAZodiac::None || Element == EDBAElement::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(FixedSkillGroupTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildSkillGroupRowName(Zodiac, Element);
	return FindDataRow(DataTable, RowName, OutRow);
}

bool UDBAZodiacHeroDataAsset::GetAbilitySetSummaryData(EDBAZodiac Zodiac, FDBAZodiacHeroAbilitySetSummaryRow& OutRow) const
{
	if (Zodiac == EDBAZodiac::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(AbilitySetSummaryTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildZodiacRowName(Zodiac);
	return FindDataRow(DataTable, RowName, OutRow);
}

void UDBAZodiacHeroDataAsset::GetAllAvailableZodiacs(TArray<EDBAZodiac>& OutZodiacs) const
{
	OutZodiacs.Empty();

	UDataTable* DataTable = LoadDataTable(ZodiacHeroDisplayTable);
	if (!DataTable)
	{
		return;
	}

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FDBAZodiacHeroDisplayRow* Row = DataTable->FindRow<FDBAZodiacHeroDisplayRow>(RowName, TEXT(""));
		if (Row && Row->bIsAvailable && !Row->bIsInDevelopment)
		{
			OutZodiacs.Add(Row->Zodiac);
		}
	}
}

bool UDBAZodiacHeroDataAsset::IsZodiacAvailable(EDBAZodiac Zodiac) const
{
	FDBAZodiacHeroDisplayRow DisplayRow;
	if (!GetZodiacHeroDisplayData(Zodiac, DisplayRow))
	{
		return false;
	}

	return DisplayRow.bIsAvailable && !DisplayRow.bIsInDevelopment;
}

bool UDBAZodiacHeroDataAsset::IsSkillGroupAvailable(EDBAZodiac Zodiac, EDBAElement Element) const
{
	FDBAZodiacElementFixedSkillGroupRow SkillGroupRow;
	if (!GetFixedSkillGroupData(Zodiac, Element, SkillGroupRow))
	{
		return false;
	}

	return SkillGroupRow.bEnabled && !SkillGroupRow.bIsInDevelopment;
}

bool UDBAZodiacHeroDataAsset::ValidateDataIntegrity(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();
	bool bIsValid = true;

	// 验证数据表引用
	if (ZodiacHeroDisplayTable.IsNull())
	{
		OutErrors.Add(TEXT("ZodiacHeroDisplayTable 未配置"));
		bIsValid = false;
	}

	if (ZodiacHeroConfigTable.IsNull())
	{
		OutErrors.Add(TEXT("ZodiacHeroConfigTable 未配置"));
		bIsValid = false;
	}

	if (FixedSkillGroupTable.IsNull())
	{
		OutErrors.Add(TEXT("FixedSkillGroupTable 未配置"));
		bIsValid = false;
	}

	if (AbilitySetSummaryTable.IsNull())
	{
		OutErrors.Add(TEXT("AbilitySetSummaryTable 未配置"));
		bIsValid = false;
	}

	// 验证数据表内容
	UDataTable* DisplayTable = LoadDataTable(ZodiacHeroDisplayTable);
	if (DisplayTable)
	{
		TArray<FName> RowNames = DisplayTable->GetRowNames();
		if (RowNames.Num() != DBAConstants::ZodiacCount)
		{
			OutErrors.Add(FString::Printf(TEXT("ZodiacHeroDisplayTable 应包含 12 行数据，实际 %d 行"), RowNames.Num()));
			bIsValid = false;
		}
	}

	UDataTable* SkillGroupTable = LoadDataTable(FixedSkillGroupTable);
	if (SkillGroupTable)
	{
		TArray<FName> RowNames = SkillGroupTable->GetRowNames();
		if (RowNames.Num() != DBAConstants::FixedSkillGroupRowCount)
		{
			OutErrors.Add(FString::Printf(TEXT("FixedSkillGroupTable 应包含 60 行数据（12 生肖 × 5 元素），实际 %d 行"), RowNames.Num()));
			bIsValid = false;
		}
	}

	return bIsValid;
}

UDataTable* UDBAZodiacHeroDataAsset::LoadDataTable(const TSoftObjectPtr<UDataTable>& DataTablePtr) const
{
	if (DataTablePtr.IsNull())
	{
		return nullptr;
	}

	return DataTablePtr.LoadSynchronous();
}

FName UDBAZodiacHeroDataAsset::BuildZodiacRowName(EDBAZodiac Zodiac) const
{
	const FString ZodiacString = UEnum::GetValueAsString(Zodiac);
	// 移除 "EDBAZodiac::" 前缀
	FString CleanZodiacString = ZodiacString;
	CleanZodiacString.RemoveFromStart(TEXT("EDBAZodiac::"));

	return FName(*FString::Printf(TEXT("Zodiac_%s"), *CleanZodiacString));
}

FName UDBAZodiacHeroDataAsset::BuildSkillGroupRowName(EDBAZodiac Zodiac, EDBAElement Element) const
{
	return BuildFixedSkillGroupRowName(Zodiac, Element);
}
