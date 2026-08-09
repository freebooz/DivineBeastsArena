// Copyright Freebooz Games, Inc. All Rights Reserved.
// 十二生肖静态配置 Registry：仅索引 Primary Asset 元数据，按需异步加载单个生肖资产。

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManagerTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAZodiacRegistrySubsystem.generated.h"

struct FStreamableHandle;
class UDBAZodiacHeroDataAsset;

DECLARE_DELEGATE_TwoParams(FDBAOnZodiacHeroAssetLoaded, EDBAZodiac /* Zodiac */, UDBAZodiacHeroDataAsset* /* Asset */);

/**
 * 唯一的单生肖静态数据入口。
 *
 * 初始化只读取 AssetRegistry 元数据，因此不会在 Boot 或 Frontend 启动阶段同步加载十二生肖的网格、动画、特效或音效。
 * 前台和预览只能通过 LoadAsync 取得当前选中生肖的资产；Dedicated Server 可查询身份配置，但不应请求前台资源 Bundle。
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAZodiacRegistrySubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 重新读取 ZodiacHero Primary Asset 元数据；不加载资产内容或其软引用资源。 */
	bool RefreshRegistry();

	/** 返回已索引的 Primary Asset Id 列表，顺序为 EDBAZodiac 枚举顺序。 */
	const TArray<FPrimaryAssetId>& GetAll();

	/** Blueprint 友好的生肖身份列表；UI 使用本接口生成列表，禁止手写 12 项。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Zodiac|Registry")
	void GetAllZodiacTypes(TArray<EDBAZodiac>& OutZodiacTypes);

	/** 查询生肖对应的 Primary Asset Id；未配置时返回 false。 */
	UFUNCTION(BlueprintPure, Category = "DBA|Zodiac|Registry")
	bool Find(EDBAZodiac Zodiac, FPrimaryAssetId& OutPrimaryAssetId) const;

	/** 异步加载单个生肖静态资产。回调可能收到 nullptr，调用方必须处理失败。 */
	bool LoadAsync(EDBAZodiac Zodiac, FDBAOnZodiacHeroAssetLoaded Completion);

	/** 释放单个生肖资产和等待回调；不得在仍使用预览资源时调用。 */
	void Release(EDBAZodiac Zodiac);

	/**
	 * 数据资产全局校验入口：检查十二生肖完整性、重复 ZodiacType、重复 PrimaryAssetId 和旧 Faction 分类迁移哨兵。
	 * 不自动执行；由编辑器数据校验流程或人工审核调用。
	 */
	bool ValidateConfiguration(TArray<FString>& OutErrors, TArray<FString>& OutWarnings);

private:
	void CompleteLoad(EDBAZodiac Zodiac, const FPrimaryAssetId& PrimaryAssetId);
	bool RefreshRegistryInternal(TArray<FString>* OutErrors, TArray<FString>* OutWarnings);

	TArray<FPrimaryAssetId> PrimaryAssetIds;
	TMap<EDBAZodiac, FPrimaryAssetId> PrimaryAssetIdByZodiac;
	TMap<EDBAZodiac, TWeakObjectPtr<UDBAZodiacHeroDataAsset>> LoadedAssets;
	TMap<EDBAZodiac, TSharedPtr<FStreamableHandle>> ActiveLoadHandles;
	TMap<EDBAZodiac, TArray<FDBAOnZodiacHeroAssetLoaded>> PendingCallbacks;
};
