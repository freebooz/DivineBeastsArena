// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Data/Assets/DBAStaticDataAsset.h"
#include "Engine/DataTable.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UDBAStaticDataAsset::UDBAStaticDataAsset()
{
	// 默认构造函数
}

void UDBAStaticDataAsset::PreloadAllTables()
{
	PreloadAllTablesAsync();
}

void UDBAStaticDataAsset::PreloadAllTablesAsync()
{
	RequestDataTableAsync(ZodiacStaticTable);
	RequestDataTableAsync(ElementDefinitionTable);
	RequestDataTableAsync(FiveCampDisplayTable);
	RequestDataTableAsync(MapDefinitionTable);
	RequestDataTableAsync(ModeDefinitionTable);
}

bool UDBAStaticDataAsset::ValidateAllTables() const
{
	// 验证所有静态数据表是否有效
	bool bAllValid = true;

	if (ZodiacStaticTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAStaticDataAsset] 生肖数据表引用为空"));
		bAllValid = false;
	}

	if (ElementDefinitionTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAStaticDataAsset] 元素数据表引用为空"));
		bAllValid = false;
	}

	if (FiveCampDisplayTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAStaticDataAsset] 阵营数据表引用为空"));
		bAllValid = false;
	}

	if (MapDefinitionTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAStaticDataAsset] 地图数据表引用为空"));
		bAllValid = false;
	}

	if (ModeDefinitionTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAStaticDataAsset] 模式数据表引用为空"));
		bAllValid = false;
	}

	return bAllValid;
}

bool UDBAStaticDataAsset::ValidateData_Implementation(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();
	bool bIsValid = true;

	// 校验软引用是否配置
	if (ZodiacStaticTable.IsNull())
	{
		OutErrors.Add(TEXT("生肖数据表引用为空"));
		bIsValid = false;
	}

	if (ElementDefinitionTable.IsNull())
	{
		OutErrors.Add(TEXT("自然元素之力数据表引用为空"));
		bIsValid = false;
	}

	if (FiveCampDisplayTable.IsNull())
	{
		OutErrors.Add(TEXT("五大阵营数据表引用为空"));
		bIsValid = false;
	}

	if (MapDefinitionTable.IsNull())
	{
		OutErrors.Add(TEXT("地图数据表引用为空"));
		bIsValid = false;
	}

	if (ModeDefinitionTable.IsNull())
	{
		OutErrors.Add(TEXT("模式数据表引用为空"));
		bIsValid = false;
	}

	// 校验已加载数据表的行数；未加载完成的表只记录提示，不计为校验失败。
	auto CheckRowCount = [this, &OutErrors, &bIsValid](
		const TSoftObjectPtr<UDataTable>& TablePtr,
		const TCHAR* TableLabel,
		int32 ExpectedRowCount,
		bool bRequireExactRowCount)
	{
		if (TablePtr.IsNull())
		{
			return;
		}

		UDataTable* Table = LoadDataTable(TablePtr);
		if (!Table)
		{
			OutErrors.Add(FString::Printf(TEXT("%s 尚未加载完成，已发起异步加载；请等待加载完成后重试校验。"), TableLabel));
			return;
		}

		const int32 RowCount = Table->GetRowNames().Num();
		if (bRequireExactRowCount)
		{
			if (RowCount != ExpectedRowCount)
			{
				OutErrors.Add(FString::Printf(TEXT("%s 行数错误：当前 %d 行，期望 %d 行。"), TableLabel, RowCount, ExpectedRowCount));
				bIsValid = false;
			}
		}
		else
		{
			if (RowCount < ExpectedRowCount)
			{
				OutErrors.Add(FString::Printf(TEXT("%s 行数不足：当前 %d 行，至少需要 %d 行。"), TableLabel, RowCount, ExpectedRowCount));
				bIsValid = false;
			}
		}
	};

	CheckRowCount(ZodiacStaticTable, TEXT("生肖数据表"), 1, false);
	CheckRowCount(ElementDefinitionTable, TEXT("自然元素之力数据表"), DBAConstants::ElementCount, true);
	CheckRowCount(FiveCampDisplayTable, TEXT("五大阵营数据表"), DBAConstants::ElementCount, true);
	CheckRowCount(MapDefinitionTable, TEXT("地图数据表"), 1, false);
	CheckRowCount(ModeDefinitionTable, TEXT("模式数据表"), 1, false);

	if (!bIsValid)
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAStaticDataAsset] 运行时数据校验失败，共 %d 项错误"), OutErrors.Num());
	}

	return bIsValid;
}

UDataTable* UDBAStaticDataAsset::GetZodiacStaticTable() const
{
	return LoadDataTable(ZodiacStaticTable);
}

UDataTable* UDBAStaticDataAsset::GetElementDefinitionTable() const
{
	return LoadDataTable(ElementDefinitionTable);
}

UDataTable* UDBAStaticDataAsset::GetFiveCampDisplayTable() const
{
	return LoadDataTable(FiveCampDisplayTable);
}

UDataTable* UDBAStaticDataAsset::GetMapDefinitionTable() const
{
	return LoadDataTable(MapDefinitionTable);
}

UDataTable* UDBAStaticDataAsset::GetModeDefinitionTable() const
{
	return LoadDataTable(ModeDefinitionTable);
}

UDataTable* UDBAStaticDataAsset::LoadDataTable(const TSoftObjectPtr<UDataTable>& DataTablePtr) const
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

void UDBAStaticDataAsset::RequestDataTableAsync(const TSoftObjectPtr<UDataTable>& DataTablePtr) const
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
	TWeakObjectPtr<UDBAStaticDataAsset> WeakThis(const_cast<UDBAStaticDataAsset*>(this));
	DBAAsyncAssetLoader::RequestAsyncAsset<UDataTable>(const_cast<UDBAStaticDataAsset*>(this), DataTablePtr, [WeakThis, TablePath](UDataTable* LoadedTable)
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		if (!LoadedTable)
		{
			UE_LOG(LogDBAData, Warning, TEXT("[UDBAStaticDataAsset] 静态数据表异步加载失败：%s"), *TablePath.ToString());
			return;
		}

		UE_LOG(LogDBAData, Log, TEXT("[UDBAStaticDataAsset] 静态数据表异步加载完成：%s"), *TablePath.ToString());

		// 广播 DataTable 加载完成事件，供 UI 等订阅方驱动更新。
		WeakThis->OnDataTableLoaded.Broadcast(LoadedTable, TablePath);
	});

	UE_LOG(LogDBAData, Verbose, TEXT("[UDBAStaticDataAsset] 已发起静态数据表异步加载：%s"), *TablePath.ToString());
}

#if WITH_EDITOR
void UDBAStaticDataAsset::ValidateLoadedTableRowCount(
	const TSoftObjectPtr<UDataTable>& DataTablePtr,
	const TCHAR* TableLabel,
	int32 ExpectedRowCount,
	bool bRequireExactRowCount,
	FDataValidationContext& Context,
	EDataValidationResult& Result) const
{
	if (DataTablePtr.IsNull())
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("%s 引用为空"), TableLabel)));
		Result = EDataValidationResult::Invalid;
		return;
	}

	UDataTable* Table = LoadDataTable(DataTablePtr);
	if (!Table)
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("%s 尚未加载完成，已发起异步加载；请等待加载完成后重试校验。"), TableLabel)));
		Result = EDataValidationResult::Invalid;
		return;
	}

	const int32 RowCount = Table->GetRowNames().Num();
	if (bRequireExactRowCount)
	{
		if (RowCount != ExpectedRowCount)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("%s 行数错误：当前 %d 行，期望 %d 行。"), TableLabel, RowCount, ExpectedRowCount)));
			Result = EDataValidationResult::Invalid;
		}
		return;
	}

	if (RowCount < ExpectedRowCount)
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("%s 行数不足：当前 %d 行，至少需要 %d 行。"), TableLabel, RowCount, ExpectedRowCount)));
		Result = EDataValidationResult::Invalid;
	}
}

EDataValidationResult UDBAStaticDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	// 验证生肖数据表
	if (ZodiacStaticTable.IsNull())
	{
		Context.AddError(FText::FromString(TEXT("生肖数据表引用为空")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		ValidateLoadedTableRowCount(ZodiacStaticTable, TEXT("生肖数据表"), 1, false, Context, Result);
	}

	// 验证自然元素之力数据表
	if (ElementDefinitionTable.IsNull())
	{
		Context.AddError(FText::FromString(TEXT("自然元素之力数据表引用为空")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		ValidateLoadedTableRowCount(ElementDefinitionTable, TEXT("自然元素之力数据表"), DBAConstants::ElementCount, true, Context, Result);
	}

	// 验证五大阵营数据表
	if (FiveCampDisplayTable.IsNull())
	{
		Context.AddError(FText::FromString(TEXT("五大阵营数据表引用为空")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		ValidateLoadedTableRowCount(FiveCampDisplayTable, TEXT("五大阵营数据表"), DBAConstants::ElementCount, true, Context, Result);
	}

	// 验证地图数据表
	if (MapDefinitionTable.IsNull())
	{
		Context.AddError(FText::FromString(TEXT("地图数据表引用为空")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		ValidateLoadedTableRowCount(MapDefinitionTable, TEXT("地图数据表"), 1, false, Context, Result);
	}

	// 验证游戏模式数据表
	if (ModeDefinitionTable.IsNull())
	{
		Context.AddError(FText::FromString(TEXT("游戏模式数据表引用为空")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		ValidateLoadedTableRowCount(ModeDefinitionTable, TEXT("游戏模式数据表"), 1, false, Context, Result);
	}

	return Result;
}
#endif
