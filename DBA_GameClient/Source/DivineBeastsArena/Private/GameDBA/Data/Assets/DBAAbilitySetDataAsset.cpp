// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Data/Assets/DBAAbilitySetDataAsset.h"
#include "Engine/DataTable.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"

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

	bIsValid &= ValidateLoadedTableRowCount(ElementPassiveTable, TEXT("ElementPassiveTable"), DBAConstants::ElementCount, TEXT("5 元素"), OutErrors);
	bIsValid &= ValidateLoadedTableRowCount(ElementActiveAbilityTable, TEXT("ElementActiveAbilityTable"), DBAConstants::ElementActiveAbilityRowCount, TEXT("5 元素 x 5 位置"), OutErrors);
	bIsValid &= ValidateLoadedTableRowCount(ElementUltimateTemplateTable, TEXT("ElementUltimateTemplateTable"), DBAConstants::ElementCount, TEXT("5 元素"), OutErrors);
	bIsValid &= ValidateLoadedTableRowCount(ElementResonanceTable, TEXT("ElementResonanceTable"), DBAConstants::ElementResonanceRowCount, TEXT("5 元素 x 5 等级"), OutErrors);
	bIsValid &= ValidateLoadedTableRowCount(ZodiacUltimateTable, TEXT("ZodiacUltimateTable"), DBAConstants::ZodiacCount, TEXT("12 生肖"), OutErrors);

	return bIsValid;
}

bool UDBAAbilitySetDataAsset::ValidateData_Implementation(TArray<FString>& OutErrors) const
{
	// 委托给已有的数据完整性校验逻辑，统一通过 IDBAValidatableInterface 暴露运行时校验入口。
	const bool bIsValid = ValidateDataIntegrity(OutErrors);
	if (!bIsValid)
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAAbilitySetDataAsset] 运行时数据校验失败，共 %d 项错误"), OutErrors.Num());
	}
	return bIsValid;
}

void UDBAAbilitySetDataAsset::PreloadAllDataTablesAsync() const
{
	RequestDataTableAsync(ElementPassiveTable);
	RequestDataTableAsync(ElementActiveAbilityTable);
	RequestDataTableAsync(ElementUltimateTemplateTable);
	RequestDataTableAsync(ElementResonanceTable);
	RequestDataTableAsync(ZodiacUltimateTable);
	RequestDataTableAsync(SkillDataTable);
	RequestDataTableAsync(AbilitySetSummaryTable);
}

UDataTable* UDBAAbilitySetDataAsset::LoadDataTable(const TSoftObjectPtr<UDataTable>& DataTablePtr) const
{
	if (DataTablePtr.IsNull())
	{
		return nullptr;
	}

	if (UDataTable* LoadedTable = DataTablePtr.Get())
	{
		return LoadedTable;
	}

	RequestDataTableAsync(DataTablePtr);
	return nullptr;
}

void UDBAAbilitySetDataAsset::RequestDataTableAsync(const TSoftObjectPtr<UDataTable>& DataTablePtr) const
{
	if (DataTablePtr.IsNull() || DataTablePtr.Get())
	{
		return;
	}

	const FSoftObjectPath TablePath = DataTablePtr.ToSoftObjectPath();
	if (RequestedDataTableLoads.Contains(TablePath))
	{
		return;
	}

	RequestedDataTableLoads.Add(TablePath);
	TWeakObjectPtr<UDBAAbilitySetDataAsset> WeakThis(const_cast<UDBAAbilitySetDataAsset*>(this));
	DBAAsyncAssetLoader::RequestAsyncAsset<UDataTable>(const_cast<UDBAAbilitySetDataAsset*>(this), DataTablePtr, [WeakThis, TablePath](UDataTable* LoadedTable)
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		if (!LoadedTable)
		{
			UE_LOG(LogDBAData, Warning, TEXT("[UDBAAbilitySetDataAsset] 技能组数据表异步加载失败：%s"), *TablePath.ToString());
			return;
		}

		UE_LOG(LogDBAData, Log, TEXT("[UDBAAbilitySetDataAsset] 技能组数据表异步加载完成：%s"), *TablePath.ToString());

		// 广播 DataTable 加载完成事件，供 UI 等订阅方驱动更新。
		WeakThis->OnDataTableLoaded.Broadcast(LoadedTable, TablePath);
	});

	UE_LOG(LogDBAData, Verbose, TEXT("[UDBAAbilitySetDataAsset] 已发起技能组数据表异步加载：%s"), *TablePath.ToString());
}

bool UDBAAbilitySetDataAsset::ValidateLoadedTableRowCount(
	const TSoftObjectPtr<UDataTable>& DataTablePtr,
	const TCHAR* TableLabel,
	int32 ExpectedRowCount,
	const TCHAR* ExpectedDescription,
	TArray<FString>& OutErrors) const
{
	if (DataTablePtr.IsNull())
	{
		return true;
	}

	UDataTable* DataTable = LoadDataTable(DataTablePtr);
	if (!DataTable)
	{
		OutErrors.Add(FString::Printf(TEXT("%s 尚未加载完成，已发起异步加载；请等待加载完成后重试校验。"), TableLabel));
		return false;
	}

	const TArray<FName> RowNames = DataTable->GetRowNames();
	if (RowNames.Num() != ExpectedRowCount)
	{
		OutErrors.Add(FString::Printf(TEXT("%s 应包含 %d 行数据（%s），实际 %d 行"), TableLabel, ExpectedRowCount, ExpectedDescription, RowNames.Num()));
		return false;
	}

	return true;
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
