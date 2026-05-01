// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DBAZodiacCharacterBase.generated.h"

class UDBAZodiacVFXComponent;
class UDBAZodiacAnimInstance;
class UDBAAbilitySystemComponent;

/**
 * DBAZodiacCharacterBase
 * 生肖角色基类
 * 提供角色公共功能：VFX/SFX播放、动画控制、属性同步
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ADBAZodiacCharacterBase();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	// ==================== 组件获取 ====================

	/** 获取VFX组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	UDBAZodiacVFXComponent* GetVFXComponent() const { return VFXComponent; }

	/** 获取动画实例 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	UDBAZodiacAnimInstance* GetZodiacAnimInstance() const;

	/** 获取能力系统组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	UDBAAbilitySystemComponent* GetDBAAbilitySystemComponent() const;

public:
	// ==================== VFX/SFX 接口 ====================

	/** 播放攻击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX")
	void PlayAttackVFX(AActor* Target);

	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX")
	void PlayHitVFX(AActor* Attacker);

	/** 播放技能特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX")
	void PlaySkillVFX(FName SkillId, AActor* Target);

	/** 播放攻击音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX")
	void PlayAttackSFX();

	/** 播放受击音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX")
	void PlayHitSFX();

	/** 播放技能音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX")
	void PlaySkillSFX(FName SkillId);

public:
	// ==================== 动画接口 ====================

	/** 播放攻击动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void PlayAttackAnimation();

	/** 播放受击动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void PlayHitAnimation();

	/** 播放死亡动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void PlayDeathAnimation();

	/** 设置移动速度 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetAnimMoveSpeed(float Speed);

protected:
	// ==================== 组件 ====================

	/** VFX/SFX管理组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Components")
	TObjectPtr<UDBAZodiacVFXComponent> VFXComponent;

	/** 角色元素类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName ElementType = FName(TEXT("Fire"));

	/** 角色生肖类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName ZodiacType = FName(TEXT("None"));
};
