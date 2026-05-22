// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "Components/SceneComponent.h"
#include "DBAFloatingDamageComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UTextRenderComponent;

/**
 * FDBAFloatingDamageEntry
 * 浮动伤害条目数据
 */
USTRUCT()
struct FDBAFloatingDamageEntry
{
	GENERATED_BODY()

	FDBAFloatingDamageEntry()
		: WorldLocation(FVector::ZeroVector)
		, Velocity(FVector::ZeroVector)
		, RemainingTime(0.0f)
		, TotalTime(0.0f)
		, Damage(0.0f)
		, Color(FLinearColor::White)
		, bIsCritical(false)
		, NiagaraComponent(nullptr)
		, TextComponent(nullptr)
	{}

	FDBAFloatingDamageEntry(FVector InLocation, float InDamage, FLinearColor InColor, bool bInCritical)
		: WorldLocation(InLocation)
		, Velocity(FVector(FMath::RandRange(-30.0f, 30.0f), FMath::RandRange(-30.0f, 30.0f), FMath::RandRange(150.0f, 200.0f)))
		, RemainingTime(1.2f)
		, TotalTime(1.2f)
		, Damage(InDamage)
		, Color(InColor)
		, bIsCritical(bInCritical)
		, NiagaraComponent(nullptr)
		, TextComponent(nullptr)
	{}

	/** 世界位置 */
	FVector WorldLocation;

	/** 飘动速度 */
	FVector Velocity;

	/** 剩余时间 */
	float RemainingTime;

	/** 总时间 */
	float TotalTime;

	/** 伤害值 */
	float Damage;

	/** 颜色 */
	FLinearColor Color;

	/** 是否暴击 */
	bool bIsCritical;

	/** Niagara组件 */
	TWeakObjectPtr<UNiagaraComponent> NiagaraComponent;
	TWeakObjectPtr<UTextRenderComponent> TextComponent;
};

/**
 * UDBAFloatingDamageComponent
 * 浮动伤害数字组件
 * 使用Niagara系统渲染浮动伤害数字
 */
UCLASS(Abstract, Blueprintable, BlueprintType, meta = (DisplayName = "DBA Floating Damage Component"))
class DIVINEBEASTSARENA_API UDBAFloatingDamageComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDBAFloatingDamageComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** 生成伤害数字 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|DamageNumber")
	void SpawnDamageNumber(float Damage, bool bIsCritical, uint8 ElementValue, FVector ImpactPoint);

	/** 设置Niagara系统类 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|DamageNumber")
	void SetDamageNumberSystem(UNiagaraSystem* InDamageNumberSystem);

	/** 清空所有伤害数字 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|DamageNumber")
	void ClearAllDamageNumbers();

	/** 设置伤害数字持续时间 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|DamageNumber")
	void SetDamageDuration(float Duration) { DamageDuration = Duration; }

protected:
	/** 创建伤害数字条目 */
	void SpawnDamageNumberEntry(FDBAFloatingDamageEntry& Entry);

	/** 更新伤害数字条目 */
	void UpdateDamageEntry(FDBAFloatingDamageEntry& Entry, float DeltaTime);

	/** 回收伤害数字条目 */
	void RecycleDamageEntry(FDBAFloatingDamageEntry& Entry);

private:
	/** Niagara系统 - 伤害数字 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Feedback|DamageNumber", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraSystem> DamageNumberSystem;

	/** 伤害数字池大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Feedback|DamageNumber", meta = (AllowPrivateAccess = "true", UIMin = 8, UIMax = 64))
	int32 PoolSize = 16;

	/** 单个伤害数字持续时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Feedback|DamageNumber", meta = (AllowPrivateAccess = "true"))
	float DamageDuration = 1.2f;

	/** 活跃的伤害数字条目 */
	UPROPERTY(Transient)
	TArray<FDBAFloatingDamageEntry> ActiveDamageEntries;

	/** 可复用的伤害数字条目 */
	UPROPERTY(Transient)
	TArray<FDBAFloatingDamageEntry> AvailableDamageEntries;

	/** 世界到屏幕变换缓存 */
	TWeakObjectPtr<APlayerController> CachedPlayerController;
};
