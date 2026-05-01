// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色动画实例基类

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacAnimInstance.generated.h"
#include "Animation/AnimInstance.h"

/**
 * DBAZodiacAnimInstance
 * 生肖角色动画实例基类
 * 提供动画状态管理和公共接口
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UDBAZodiacAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

public:
	// ==================== 动画状态接口 ====================

	/** 设置移动速度 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetMoveSpeed(float Speed) { MoveSpeed = Speed; }

	/** 获取移动速度 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	float GetMoveSpeed() const { return MoveSpeed; }

	/** 设置是否在移动 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetIsMoving(bool bInMoving) { bIsMoving = bInMoving; }

	/** 获取是否在移动 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	bool IsMoving() const { return bIsMoving; }

	/** 设置是否攻击中 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetIsAttacking(bool bInAttacking) { bIsAttacking = bInAttacking; }

	/** 获取是否攻击中 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	bool IsAttacking() const { return bIsAttacking; }

	/** 设置是否受击中 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetIsHit(bool bInHit) { bIsHit = bInHit; }

	/** 获取是否受击中 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	bool IsHit() const { return bIsHit; }

	/** 设置是否死亡 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetIsDead(bool bInDead) { bIsDead = bInDead; }

	/** 获取是否死亡 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	bool IsDead() const { return bIsDead; }

	/** 设置当前技能ID */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetCurrentSkillId(FName InSkillId) { CurrentSkillId = InSkillId; }

	/** 获取当前技能ID */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	FName GetCurrentSkillId() const { return CurrentSkillId; }

public:
	// ==================== 动画曲线/变量 ====================

	/** 移动速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	float MoveSpeed = 0.0f;

	/** 是否在移动 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	bool bIsMoving = false;

	/** 是否在攻击中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	bool bIsAttacking = false;

	/** 是否受击 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	bool bIsHit = false;

	/** 是否死亡 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	bool bIsDead = false;

	/** 当前技能ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	FName CurrentSkillId = NAME_None;

	/** 角色朝向 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	FRotator RotationRate = FRotator::ZeroRotator;

protected:
	/** 所属Pawn引用 */
	UPROPERTY()
	TObjectPtr<APawn> OwningPawn;
};
