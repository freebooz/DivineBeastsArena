// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// GameMoba - 通用MOBA GameplayAbility基类

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DBAMobaGameplayAbilityBase.generated.h"

/**
 * UDBAMobaGameplayAbilityBase
 * MOBA游戏通用GameplayAbility基类
 * 提供技能通用接口
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEMOBA_API UDBAMobaGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDBAMobaGameplayAbilityBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin UGameplayAbility Interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End UGameplayAbility Interface

public:
	/** 获取技能图标 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	UTexture2D* GetAbilityIcon() const { return AbilityIcon; }

	/** 获取技能名称 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	FText GetAbilityName() const { return AbilityName; }

	/** 获取冷却时间 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	float GetCooldownDuration() const { return CooldownDuration; }

	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	float GetEnergyCost() const { return EnergyCost; }

protected:
	/** 技能图标 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	TObjectPtr<UTexture2D> AbilityIcon;

	/** 技能显示名称 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	FText AbilityName;

	/** 冷却时间(秒) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	float CooldownDuration = 0.0f;

	/** 能量消耗 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	float EnergyCost = 0.0f;

	/** 是否是终极技能 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	bool bIsUltimate = false;

	/** 技能所属元素 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	FName ElementType = NAME_None;
};
