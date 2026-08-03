// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：英雄数值平衡数据 Subsystem，负责异步加载 DataTable 并提供查询接口。
- 阅读重点：InitializeWithSettings 异步加载，GetHeroBalanceData 查询返回业务结构体。
- 修改提示：新增查询接口时保持非阻塞语义，未就绪时返回默认值并输出中文日志。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "GameDBA/Gameplay/Progression/Balance/DBAAbilityBalance.h"
#include "DBAHeroBalanceSubsystem.generated.h"

class UDataTable;
struct FDBAHeroBalanceRow;

/**
 * UDBAHeroBalanceSubsystem
 *
 * 英雄数值平衡数据 Subsystem。
 *
 * 职责：
 * - 从 UDBAHeroBalanceDeveloperSettings 读取默认 DataTable 软引用。
 * - 异步加载 DataTable 并缓存。
 * - 提供按生肖类型查询英雄平衡数据的接口。
 *
 * 使用方式：
 * - GameInstance 初始化时调用 InitializeWithSettings()。
 * - 业务侧通过 GetHeroBalanceData(ZodiacType) 查询。
 * - 数据未就绪时返回默认 FDBAHeroBalanceData 并输出中文警告日志。
 *
 * 以 DataTable 取代已清理的 CDO 硬编码平衡数据，实现单一数据源驱动。
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAHeroBalanceSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

protected:
	// P1-1 改造：重写项目基类生命周期钩子，替代原生 Initialize/Deinitialize
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;

	/**
	 * 根据生肖类型获取英雄平衡数据。
	 *
	 * @param ZodiacType 生肖类型
	 * @return 英雄平衡数据；若 DataTable 未加载完成或行未找到，返回默认值并输出中文警告日志。
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Hero Balance")
	FDBAHeroBalanceData GetHeroBalanceData(EDBAZodiacType ZodiacType) const;

	/**
	 * 获取所有英雄平衡数据。
	 *
	 * @param OutData 输出数据数组
	 * @return 是否成功获取（DataTable 已加载且非空）
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Hero Balance")
	bool GetAllHeroBalanceData(TArray<FDBAHeroBalanceData>& OutData) const;

	/** 检查 DataTable 是否已加载完成。 */
	UFUNCTION(BlueprintPure, Category = "DBA|Hero Balance")
	bool IsDataTableReady() const;

protected:
	/** 从 DeveloperSettings 读取配置并发起异步加载。 */
	void InitializeWithSettings();

	/** 异步加载完成回调。 */
	void HandleDataTableLoaded();

	/** 根据 ZodiacType 查找行数据。 */
	const FDBAHeroBalanceRow* FindRowByZodiac(EDBAZodiacType ZodiacType) const;

private:
	/** 英雄平衡 DataTable 软引用（来自 DeveloperSettings）。 */
	TSoftObjectPtr<UDataTable> HeroBalanceTablePtr;

	/** 已加载的 DataTable 缓存（弱指针，避免阻止 GC）。 */
	TWeakObjectPtr<UDataTable> CachedDataTable;

	/** 异步加载是否已发起（避免重复请求）。 */
	bool bHasRequestedAsyncLoad = false;

	/** 是否已输出过"DataTable 未就绪"警告日志。 */
	mutable bool bHasLoggedNotReady = false;
};
