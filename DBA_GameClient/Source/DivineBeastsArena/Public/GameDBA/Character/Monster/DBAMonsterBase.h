// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 怪物模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DBAMonsterBase.generated.h"

/**
 * DBAMonsterBase
 * 怪物模型基类
 * 提供怪物公共功能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ADBAMonsterBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	virtual void OnRep_CurrentHealth();

public:
	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	void PlayHitVFX(AActor* Attacker);

	/** 播放死亡特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	void PlayDeathVFX();

	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	float GetHealthPercent() const;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastShowDamageNumber(float DamageAmount, FVector_NetQuantize ImpactPoint, bool bIsCritical);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName MonsterType = FName(TEXT("None"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, BlueprintReadOnly, Category = "DBA|State")
	float CurrentHealth = 100.0f;
};
