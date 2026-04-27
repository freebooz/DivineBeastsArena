// Copyright FreeboozStudio. All Rights Reserved.

#include "Data/DBAAbilitySetDataAsset.h"
#include "Engine/DataTable.h"

UDBAAbilitySetDataAsset::UDBAAbilitySetDataAsset()
{
}

bool UDBAAbilitySetDataAsset::GetElementPassiveData(EDBAElement Element, FName& OutRowName) const
{
	if (Element == EDBAElement::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(ElementPassiveTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildElementRowName(Element);
	OutRowName = RowName;
	return true;
}

bool UDBAAbilitySetDataAsset::GetElementActiveAbilityData(EDBAElement Element, int32 SkillPosition, FName& OutRowName) const
{
	if (Element == EDBAElement::None || SkillPosition < 0 || SkillPosition > 4)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(ElementActiveAbilityTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildElementSkillRowName(Element, SkillPosition);
	OutRowName = RowName;
	return true;
}

bool UDBAAbilitySetDataAsset::GetElementUltimateTemplateData(EDBAElement Element, FName& OutRowName) const
{
	if (Element == EDBAElement::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(ElementUltimateTemplateTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildElementRowName(Element);
	OutRowName = RowName;
	return true;
}

bool UDBAAbilitySetDataAsset::GetElementResonanceData(EDBAElement Element, int32 ResonanceLevel, FName& OutRowName) const
{
	if (Element == EDBAElement::None || ResonanceLevel < 0 || ResonanceLevel > 4)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(ElementResonanceTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildResonanceRowName(Element, ResonanceLevel);
	OutRowName = RowName;
	return true;
}

bool UDBAAbilitySetDataAsset::GetZodiacUltimateData(EDBAZodiac Zodiac, FName& OutRowName) const
{
	if (Zodiac == EDBAZodiac::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(ZodiacUltimateTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildZodiacRowName(Zodiac);
	OutRowName = RowName;
	return true;
}

bool UDBAAbilitySetDataAsset::GetAbilitySetSummaryData(EDBAZodiac Zodiac, FName& OutRowName) const
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
	OutRowName = RowName;
	return true;
}

bool UDBAAbilitySetDataAsset::ValidateDataIntegrity(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();
	bool bIsValid = true;

	// 验证数据表引用
	if (ElementPassiveTable.IsNull())
	{
		OutErrors.Add(TEXT("ElementPassiveTable 未配置"));
		bIsValid = false;
	}

	if (ElementActiveAbilityTable.IsNull())
	{
		OutErrors.Add(TEXT("ElementActiveAbilityTable 未配置"));
		bIsValid = false;
	}

	if (ElementUltimateTemplateTable.IsNull())
	{
		OutErrors.Add(TEXT("ElementUltimateTemplateTable 未配置"));
		bIsValid = false;
	}

	if (ElementResonanceTable.IsNull())
	{
		OutErrors.Add(TEXT("ElementResonanceTable 未配置"));
		bIsValid = false;
	}

	if (ZodiacUltimateTable.IsNull())
	{
		OutErrors.Add(TEXT("ZodiacUltimateTable 未配置"));
		bIsValid = false;
	}

	if (SkillDataTable.IsNull())
	{
		OutErrors.Add(TEXT("SkillDataTable 未配置"));
		bIsValid = false;
	}

	if (AbilitySetSummaryTable.IsNull())
	{
		OutErrors.Add(TEXT("AbilitySetSummaryTable 未配置"));
		bIsValid = false;
	}

	// 验证数据表内容
	UDataTable* PassiveTable = LoadDataTable(ElementPassiveTable);
	if (PassiveTable)
	{
		TArray<FName> RowNames = PassiveTable->GetRowNames();
		if (RowNames.Num() != 5)
		{
			OutErrors.Add(FString::Printf(TEXT("ElementPassiveTable 应包含 5 行数据（5 元素），实际 %d 行"), RowNames.Num()));
			bIsValid = false;
		}
	}

	UDataTable* ActiveTable = LoadDataTable(ElementActiveAbilityTable);
	if (ActiveTable)
	{
		TArray<FName> RowNames = ActiveTable->GetRowNames();
		if (RowNames.Num() != 25)
		{
			OutErrors.Add(FString::Printf(TEXT("ElementActiveAbilityTable 应包含 25 行数据（5元素×5位置），实际 %d 行"), RowNames.Num()));
			bIsValid = false;
		}
	}

	UDataTable* UltimateTable = LoadDataTable(ElementUltimateTemplateTable);
	if (UltimateTable)
	{
		TArray<FName> RowNames = UltimateTable->GetRowNames();
		if (RowNames.Num() != 5)
		{
			OutErrors.Add(FString::Printf(TEXT("ElementUltimateTemplateTable 应包含 5 行数据（5 元素），实际 %d 行"), RowNames.Num()));
			bIsValid = false;
		}
	}

	UDataTable* ResonanceTableData = LoadDataTable(ElementResonanceTable);
	if (ResonanceTableData)
	{
		TArray<FName> RowNames = ResonanceTableData->GetRowNames();
		if (RowNames.Num() != 25)
		{
			OutErrors.Add(FString::Printf(TEXT("ElementResonanceTable 应包含 25 行数据（5元素×5等级），实际 %d 行"), RowNames.Num()));
			bIsValid = false;
		}
	}

	UDataTable* ZodiacUltimateTableData = LoadDataTable(ZodiacUltimateTable);
	if (ZodiacUltimateTableData)
	{
		TArray<FName> RowNames = ZodiacUltimateTableData->GetRowNames();
		if (RowNames.Num() != 12)
		{
			OutErrors.Add(FString::Printf(TEXT("ZodiacUltimateTable 应包含 12 行数据（12 生肖），实际 %d 行"), RowNames.Num()));
			bIsValid = false;
		}
	}

	return bIsValid;
}

UDataTable* UDBAAbilitySetDataAsset::LoadDataTable(const TSoftObjectPtr<UDataTable>& DataTablePtr) const
{
	if (DataTablePtr.IsNull())
	{
		return nullptr;
	}

	return DataTablePtr.LoadSynchronous();
}

FName UDBAAbilitySetDataAsset::BuildElementRowName(EDBAElement Element) const
{
	const FString ElementString = UEnum::GetValueAsString(Element);
	FString CleanElementString = ElementString;
	CleanElementString.RemoveFromStart(TEXT("EDBAElement::"));

	return FName(*FString::Printf(TEXT("Element_%s"), *CleanElementString));
}

FName UDBAAbilitySetDataAsset::BuildElementSkillRowName(EDBAElement Element, int32 Position) const
{
	const FString ElementString = UEnum::GetValueAsString(Element);
	FString CleanElementString = ElementString;
	CleanElementString.RemoveFromStart(TEXT("EDBAElement::"));

	return FName(*FString::Printf(TEXT("Element_%s_Skill_%d"), *CleanElementString, Position));
}

FName UDBAAbilitySetDataAsset::BuildResonanceRowName(EDBAElement Element, int32 ResonanceLevel) const
{
	const FString ElementString = UEnum::GetValueAsString(Element);
	FString CleanElementString = ElementString;
	CleanElementString.RemoveFromStart(TEXT("EDBAElement::"));

	return FName(*FString::Printf(TEXT("Element_%s_Resonance_%d"), *CleanElementString, ResonanceLevel));
}

FName UDBAAbilitySetDataAsset::BuildZodiacRowName(EDBAZodiac Zodiac) const
{
	const FString ZodiacString = UEnum::GetValueAsString(Zodiac);
	FString CleanZodiacString = ZodiacString;
	CleanZodiacString.RemoveFromStart(TEXT("EDBAZodiac::"));

	return FName(*FString::Printf(TEXT("Zodiac_%s"), *CleanZodiacString));
}
