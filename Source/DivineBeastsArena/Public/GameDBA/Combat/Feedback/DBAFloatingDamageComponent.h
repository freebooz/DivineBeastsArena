// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "Components/SceneComponent.h"
#include "NiagaraSystem.h"
#include "DBAFloatingDamageComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

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
	void SetDamageNumberSystem(TSubclassOf<UNiagaraSystem> InDamageNumberSystem);

	/** 清空所有伤害数字 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|DamageNumber")
	void ClearAllDamageNumbers();

	/** 设置伤害数字持续时间 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|DamageNumber")
	void SetDamageDuration(float Duration) { DamageDuration = Duration; }

protected:
	/** 创建伤害数字条目 */
	void SpawnDamageNumberEntry(const FDBAFloatingDamageEntry& Entry);

	/** 更新伤害数字条目 */
	void UpdateDamageEntry(FDBAFloatingDamageEntry& Entry, float DeltaTime);

	/** 回收伤害数字条目 */
	void RecycleDamageEntry(FDBAFloatingDamageEntry& Entry);

private:
	/** Niagara系统 - 伤害数字 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Feedback|DamageNumber", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UNiagaraSystem> DamageNumberSystem;

	/** 伤害数字池大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Feedback|DamageNumber", meta = (AllowPrivateAccess = "true", UIMin = 8, UIMax = 64))
	int32 PoolSize = 16;

	/** 单个伤害数字持续时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Feedback|DamageNumber", meta = (AllowPrivateAccess = "true"))
	float DamageDuration = 1.2f;

	/** 活跃的伤害数字条目 */
	UPROPERTY()
	TArray<FDBAFloatingDamageEntry> ActiveDamageEntries;

	/** 可复用的伤害数字条目 */
	UPROPERTY()
	TArray<FDBAFloatingDamageEntry> AvailableDamageEntries;

	/** 世界到屏幕变换缓存 */
	TWeakObjectPtr<APlayerController> CachedPlayerController;
};