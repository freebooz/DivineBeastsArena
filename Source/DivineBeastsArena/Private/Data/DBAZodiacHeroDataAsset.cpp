// Copyright FreeboozStudio. All Rights Reserved.

#include "Data/DBAZodiacHeroDataAsset.h"
#include "Engine/DataTable.h"

UDBAZodiacHeroDataAsset::UDBAZodiacHeroDataAsset()
{
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
		if (RowNames.Num() != 12)
		{
			OutErrors.Add(FString::Printf(TEXT("ZodiacHeroDisplayTable 应包含 12 行数据，实际 %d 行"), RowNames.Num()));
			bIsValid = false;
		}
	}

	UDataTable* SkillGroupTable = LoadDataTable(FixedSkillGroupTable);
	if (SkillGroupTable)
	{
		TArray<FName> RowNames = SkillGroupTable->GetRowNames();
		if (RowNames.Num() != 60)
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
	const FString ZodiacString = UEnum::GetValueAsString(Zodiac);
	const FString ElementString = UEnum::GetValueAsString(Element);

	// 移除枚举前缀
	FString CleanZodiacString = ZodiacString;
	CleanZodiacString.RemoveFromStart(TEXT("EDBAZodiac::"));

	FString CleanElementString = ElementString;
	CleanElementString.RemoveFromStart(TEXT("EDBAElement::"));

	return FName(*FString::Printf(TEXT("Zodiac_%s_Element_%s"), *CleanZodiacString, *CleanElementString));
}
