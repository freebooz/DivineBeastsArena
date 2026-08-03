// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：DataTable 异步加载工具结构体实现。
- 阅读重点：LoadOrRequestAsync 三段式逻辑（已加载→发起异步→等待），HandleAsyncLoadComplete 广播完成事件。
- 修改提示：保持非阻塞语义，失败时输出中文错误日志。
*/

#include "GameCore/Async/DBAAsyncDataTableLoader.h"

#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogDBAAsyncDataTableLoader, Log, All);

UDataTable* FDBAAsyncDataTableLoader::LoadOrRequestAsync(const TSoftObjectPtr<UDataTable>& DataTablePtr, const TCHAR* LoaderContext)
{
	// 若软引用无效，直接返回 nullptr。
	if (!DataTablePtr.ToSoftObjectPath().IsValid())
	{
		UE_LOG(LogDBAAsyncDataTableLoader, Warning, TEXT("[AsyncDataTableLoader] 软引用路径无效，无法加载。上下文：%s"),
			LoaderContext ? LoaderContext : TEXT("未知"));
		return nullptr;
	}

	// 若软引用已加载，直接返回并广播 OnLoaded。
	if (DataTablePtr.IsValid())
	{
		UDataTable* Table = DataTablePtr.Get();
		if (Table)
		{
			OnLoaded.Broadcast(Table);
			return Table;
		}
	}

	// 软引用未加载，检查是否已发起过异步请求。
	const FSoftObjectPath& Path = DataTablePtr.ToSoftObjectPath();
	if (RequestedPaths.Contains(Path))
	{
		// 已发起过请求，等待完成。
		return nullptr;
	}

	// 发起异步加载。
	RequestAsync(DataTablePtr, LoaderContext);
	return nullptr;
}

void FDBAAsyncDataTableLoader::RequestAsync(const TSoftObjectPtr<UDataTable>& DataTablePtr, const TCHAR* LoaderContext)
{
	const FSoftObjectPath& Path = DataTablePtr.ToSoftObjectPath();
	if (!Path.IsValid())
	{
		UE_LOG(LogDBAAsyncDataTableLoader, Warning, TEXT("[AsyncDataTableLoader] RequestAsync 软引用路径无效。上下文：%s"),
			LoaderContext ? LoaderContext : TEXT("未知"));
		return;
	}

	if (RequestedPaths.Contains(Path))
	{
		// 已发起过请求，跳过。
		return;
	}

	RequestedPaths.Add(Path);

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(
		Path,
		FStreamableDelegate::CreateRaw(this, &FDBAAsyncDataTableLoader::HandleAsyncLoadComplete, DataTablePtr, LoaderContext));

	UE_LOG(LogDBAAsyncDataTableLoader, Verbose, TEXT("[AsyncDataTableLoader] 已发起异步加载请求：%s（上下文：%s）"),
		*Path.ToString(),
		LoaderContext ? LoaderContext : TEXT("未知"));
}

UDataTable* FDBAAsyncDataTableLoader::GetLoaded(const TSoftObjectPtr<UDataTable>& DataTablePtr) const
{
	if (!DataTablePtr.IsValid())
	{
		return nullptr;
	}

	return DataTablePtr.Get();
}

void FDBAAsyncDataTableLoader::Reset()
{
	if (StreamableHandle.IsValid())
	{
		StreamableHandle->CancelHandle();
		StreamableHandle.Reset();
	}

	RequestedPaths.Reset();
	OnLoaded.Clear();
}

void FDBAAsyncDataTableLoader::HandleAsyncLoadComplete(TSoftObjectPtr<UDataTable> DataTablePtr, const TCHAR* LoaderContext)
{
	StreamableHandle.Reset();

	UDataTable* LoadedTable = DataTablePtr.Get();
	if (!LoadedTable)
	{
		UE_LOG(LogDBAAsyncDataTableLoader, Warning, TEXT("[AsyncDataTableLoader] 异步加载完成但解析为空，请检查软引用路径：%s（上下文：%s）"),
			*DataTablePtr.ToSoftObjectPath().ToString(),
			LoaderContext ? LoaderContext : TEXT("未知"));
	}
	else
	{
		UE_LOG(LogDBAAsyncDataTableLoader, Log, TEXT("[AsyncDataTableLoader] DataTable 异步加载完成：%s（上下文：%s，行数：%d）"),
			*LoadedTable->GetName(),
			LoaderContext ? LoaderContext : TEXT("未知"),
			LoadedTable->GetRowMap().Num());
	}

	// 从已请求集合中移除（允许后续重新请求）。
	RequestedPaths.Remove(DataTablePtr.ToSoftObjectPath());

	// 广播完成事件。
	OnLoaded.Broadcast(LoadedTable);
}
