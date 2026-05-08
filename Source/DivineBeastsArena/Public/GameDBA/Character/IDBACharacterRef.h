// Copyright Freebooz Games, Inc. All Rights Reserved.
// 角色引用接口 - 抽象 RPC Handler 所需的角色操作

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "IDBACharacterRef.generated.h"

class UAbilitySystemComponent;

/**
 * IDBACharacterRef
 * 角色引用接口 - 解除 ADBARpcHandler 与 ADBAZodiacCharacterBase 的直接耦合
 *
 * 使用 TScriptInterface<IDBACharacterRef> 让 RPC Handler 可以通过接口访问角色属性，
 * 而无需直接依赖具体角色类。
 */
UINTERFACE(BlueprintType)
class DIVINEBEASTSARENA_API UIDBACharacterRef : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBACharacterRef
{
	GENERATED_BODY()

public:
	/** 获取当前生命值 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual float GetCurrentHealth() const = 0;

	/** 获取最大生命值 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual float GetMaxHealth() const = 0;

	/** 获取当前能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual float GetCurrentEnergy() const = 0;

	/** 获取最大能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual float GetMaxEnergy() const = 0;

	/** 获取终极能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual float GetUltimateEnergy() const = 0;

	/** 获取连锁等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual int32 GetChainLevel() const = 0;

	/** 获取共鸣等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual int32 GetResonanceLevel() const = 0;

	/** 获取队伍ID */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual int32 GetTeamID() const = 0;

	/** 获取元素类型 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual EDBAElementType GetElementType() const = 0;

	/** 获取 AbilitySystemComponent */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const = 0;

	/** 是否已死亡 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual bool IsDead() const = 0;

	/** 是否在技能冷却中 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual bool IsAbilityOnCooldown(FName SkillId) const = 0;

	/** 是否有足够的能量释放技能 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	virtual bool HasEnoughEnergy(float Cost) const = 0;
};
