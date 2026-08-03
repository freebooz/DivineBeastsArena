// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明 DataTable 异步加载工具结构体，封装加载、防重复请求和完成事件广播。
- 阅读重点：FDBAAsyncDataTableLoader 是非 UCLASS 的纯 C++ 工具结构，供 DataAsset 类复用。
- 修改提示：保持非阻塞语义，加载完成时广播 OnLoaded 委托，失败时输出中文错误日志。
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "Engine/DataTable.h"
#include "DBAAsyncDataTableLoader.generated.h"

class UDataTable;
struct FStreamableHandle;

/**
 * FDBAAsyncDataTableLoader
 *
 * DataTable 异步加载工具结构体。
 *
 * 设计目的：
 * - 统一 UDBAStaticDataAsset / UDBAZodiacHeroDataAsset / UDBAAbilitySetDataAsset 等数据资产类的异步加载模式。
 * - 提供 OnLoaded 完成事件广播，解决旧 LoadDataTable/RequestDataTableAsync 模式缺少事件通知的问题。
 * - 防重复请求，避免对同一软引用发起多次异步加载。
 *
 * 使用方式：
 *   FDBAAsyncDataTableLoader Loader;
 *   Loader.OnLoaded.AddRaw(this, &FMyClass::HandleTableLoaded);
 *   UDataTable* Table = Loader.LoadOrRequestAsync(SoftTablePtr);
 *   // 首次返回 nullptr，加载完成后回调 OnLoaded
 *
 * 注意：
 * - 本结构体非 UCLASS，无法直接持有 UPROPERTY，但可作为 USTRUCT 嵌入其他 UCLASS 的成员。
 * - OnLoaded 委托使用 TMulticastDelegate，支持多订阅者。
 * - 持有本结构体的 UCLASS 需在销毁时调用 Reset() 取消未完成的异步加载句柄。
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBAAsyncDataTableLoader
{
	GENERATED_BODY()

public:
	/** DataTable 加载完成事件。参数为加载完成的 DataTable 指针（可能为空，表示加载失败）。 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDataTableLoaded, UDataTable*);

	/** 加载完成事件广播。 */
	FOnDataTableLoaded OnLoaded;

	/**
	 * 加载或请求异步加载 DataTable。
	 *
	 * - 若软引用已加载，直接返回 UDataTable* 并广播 OnLoaded。
	 * - 若软引用未加载且未发起过异步请求，发起异步加载并在完成时广播 OnLoaded，本次返回 nullptr。
	 * - 若软引用未加载且已发起过异步请求，直接返回 nullptr（等待异步完成）。
	 *
	 * @param DataTablePtr 数据表软引用
	 * @param OutLoaderContext 可选的加载上下文，用于日志标识
	 * @return 已加载的 UDataTable*，未加载或加载中返回 nullptr
	 */
	UDataTable* LoadOrRequestAsync(const TSoftObjectPtr<UDataTable>& DataTablePtr, const TCHAR* OutLoaderContext = TEXT(""));

	/** 直接请求异步加载（不检查是否已加载）。 */
	void RequestAsync(const TSoftObjectPtr<UDataTable>& DataTablePtr, const TCHAR* LoaderContext = TEXT(""));

	/**
	 * 获取已加载的 DataTable（不发起异步请求）。
	 *
	 * @param DataTablePtr 数据表软引用
	 * @return 已加载的 UDataTable*，未加载返回 nullptr
	 */
	UDataTable* GetLoaded(const TSoftObjectPtr<UDataTable>& DataTablePtr) const;

	/** 检查是否正在异步加载中。 */
	bool IsLoading() const { return StreamableHandle.IsValid(); }

	/** 重置状态，取消未完成的异步加载句柄。 */
	void Reset();

private:
	/** 异步加载句柄。 */
	TSharedPtr<FStreamableHandle> StreamableHandle;

	/** 已发起异步加载的软引用路径集合（防重复请求）。 */
	TSet<FSoftObjectPath> RequestedPaths;

	/** 异步加载完成回调内部处理。 */
	void HandleAsyncLoadComplete(TSoftObjectPtr<UDataTable> DataTablePtr, const TCHAR* LoaderContext);
};
