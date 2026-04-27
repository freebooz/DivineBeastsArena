// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Subsystems/DBAWorldSubsystemBase.h"
#include "Common/ObjectPool/DBAObjectPoolTypes.h"

#include "DBAObjectPoolSubsystem.generated.h"

/**
 * 对象池子系统
 *
 * 功能：
 * - 管理多种类型的对象池
 * - 提供对象获取、归还接口
 * - 支持动态扩展
 * - 提供统计信息
 * - 支持 Android 内存优化
 *
 * 适合池化的对象：
 * - Projectile：抛射物，频繁创建销毁
 * - DamageNumber：伤害数字，大量短生命周期
 * - OverheadBar：头顶血条，频繁显示隐藏
 * - Notification：通知，短暂显示
 * - CombatText：战斗文本，大量短生命周期
 * - Widget：通用 Widget，频繁创建销毁
 * - AudioComponent：音效组件，频繁播放
 * - NiagaraComponent：粒子特效，频繁播放
 *
 * 不适合池化的对象：
 * - Character / Pawn：生命周期长，状态复杂
 * - PlayerController：单例，不需要池化
 * - GameMode / GameState：单例，不需要池化
 * - 大型 Actor：内存占用大，池化收益低
 * - 有复杂依赖的对象：重置成本高
 *
 * Dedicated Server：
 * - 不创建 UI Widget 池
 * - 不创建纯表现对象池（DamageNumber / CombatText）
 * - 只创建游戏逻辑相关池（Projectile）
 *
 * Android：
 * - 根据 AndroidPoolSizeScale 缩减池大小
 * - Widget 池优先级最高，其他池可以更激进地缩减
 *
 * 线程安全：
 * - 所有接口必须在 GameThread 调用
 * - 不支持多线程并发获取/归还
 *
 * 生命周期：
 * - 随 World 创建而创建
 * - 随 World 销毁而销毁
 * - 关卡切换时会清空所有池
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAObjectPoolSubsystem : public UDBAWorldSubsystemBase
{
	GENERATED_BODY()

public:
	UDBAObjectPoolSubsystem();

	// UDBAWorldSubsystemBase interface
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;
	virtual bool IsSupportedInCurrentEnvironment() const override;
	// End of UDBAWorldSubsystemBase interface

	/**
	 * 注册对象池
	 *
	 * @param Config 对象池配置
	 * @return 是否注册成功
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	bool RegisterObjectPool(const FDBAObjectPoolConfig& Config);

	/**
	 * 注销对象池
	 *
	 * @param PoolType 池类型
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void UnregisterObjectPool(EDBAObjectPoolType PoolType);

	/**
	 * 从对象池获取对象
	 *
	 * @param PoolType 池类型
	 * @return 池化对象，如果池不存在或无可用对象则返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	UObject* AcquireObject(EDBAObjectPoolType PoolType);

	/**
	 * 归还对象到对象池
	 *
	 * @param PoolType 池类型
	 * @param Object 要归还的对象
	 * @return 是否归还成功
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	bool ReturnObject(EDBAObjectPoolType PoolType, UObject* Object);

	/**
	 * 获取对象池统计信息
	 *
	 * @param PoolType 池类型
	 * @param OutStats 输出统计信息
	 * @return 是否获取成功
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	bool GetPoolStats(EDBAObjectPoolType PoolType, FDBAObjectPoolStats& OutStats) const;

	/**
	 * 清空对象池
	 *
	 * @param PoolType 池类型
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ClearPool(EDBAObjectPoolType PoolType);

	/**
	 * 清空所有对象池
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ClearAllPools();

	/**
	 * 预热对象池
	 * 创建初始对象，避免运行时卡顿
	 *
	 * @param PoolType 池类型
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void WarmupPool(EDBAObjectPoolType PoolType);

	/**
	 * 预热所有对象池
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void WarmupAllPools();

	/**
	 * 检查对象池是否存在
	 *
	 * @param PoolType 池类型
	 * @return 是否存在
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	bool HasPool(EDBAObjectPoolType PoolType) const;

	/**
	 * 获取所有已注册的池类型
	 *
	 * @param OutPoolTypes 输出池类型列表
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void GetAllPoolTypes(TArray<EDBAObjectPoolType>& OutPoolTypes) const;

private:
	/**
	 * 所有对象池
	 * Key: 池类型
	 * Value: 池数据
	 */
	UPROPERTY()
	TMap<EDBAObjectPoolType, FObjectPoolData> ObjectPools;

	/**
	 * 创建新对象
	 *
	 * @param PoolData 池数据
	 * @return 新创建的对象
	 */
	UObject* CreateNewObject(FObjectPoolData& PoolData);

	/**
	 * 检查对象是否有效
	 *
	 * @param Object 要检查的对象
	 * @return 是否有效
	 */
	bool IsObjectValid(UObject* Object) const;

	/**
	 * 应用 Android 池大小缩放
	 *
	 * @param OriginalSize 原始大小
	 * @param Scale 缩放比例
	 * @return 缩放后的大小
	 */
	int32 ApplyAndroidPoolSizeScale(int32 OriginalSize, float Scale) const;

	/**
	 * 检查是否在 Android 平台
	 *
	 * @return 是否在 Android 平台
	 */
	bool IsAndroidPlatform() const;
};
