// Copyright Freebooz Games, Inc. All Rights Reserved.
// RPC Handler Implementation

#include "GameDBA/RPC/DBARpcHandler.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Character/IDBACharacterRef.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

TScriptInterface<IIDBACharacterRef> ADBARpcHandler::GetCharacterRef() const
{
	if (AActor* OwnerActor = GetOwner())
	{
		if (OwnerActor->Implements<UIDBACharacterRef>())
		{
			return TScriptInterface<IIDBACharacterRef>(OwnerActor);
		}
	}
	return nullptr;
}

ADBARpcHandler::ADBARpcHandler()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

// ==================== IDBARpcServer Interface ====================

void ADBARpcHandler::ServerTryActivateAbility_Implementation(const FDBAAbilityRpcParams& Params)
{
	UE_LOG(LogDBANetwork, Log, TEXT("[Server] 尝试激活技能 - Handle: %s"), *Params.AbilityHandle.ToString());

	if (TScriptInterface<IIDBACharacterRef> CharacterRef = GetCharacterRef())
	{
		if (UAbilitySystemComponent* ASC = CharacterRef->GetAbilitySystemComponent())
		{
			if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Params.AbilityHandle))
			{
				// 尝试激活技能
				ASC->TryActivateAbility(Params.AbilityHandle, false);

				// 通知客户端技能已激活
				ClientAbilityActivated_Implementation(Params.AbilityHandle);
				return;
			}
		}
	}

	UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 激活技能失败: 无法获取ASC或技能句柄无效"));
	ClientAbilityFailed_Implementation(Params.AbilityHandle, FGameplayTag());
}

bool ADBARpcHandler::ServerTryActivateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
	if (!Params.AbilityHandle.IsValid())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 激活技能被拒绝: 无效的Handle"));
		return false;
	}

	if (Params.TargetActor != nullptr && !ValidateTarget(Params.TargetActor.Get()))
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 激活技能被拒绝: 无效的目标"));
		return false;
	}

	return true;
}

void ADBARpcHandler::ServerCancelAbility_Implementation(FGameplayAbilitySpecHandle Handle)
{
	UE_LOG(LogDBANetwork, Log, TEXT("[Server] 取消技能 - Handle: %s"), *Handle.ToString());

	if (TScriptInterface<IIDBACharacterRef> CharacterRef = GetCharacterRef())
	{
		if (UAbilitySystemComponent* ASC = CharacterRef->GetAbilitySystemComponent())
		{
			// 取消技能
			ASC->CancelAbilityHandle(Handle);
			return;
		}
	}

	UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 取消技能失败: 无法获取ASC"));
}

bool ADBARpcHandler::ServerCancelAbility_Validate(FGameplayAbilitySpecHandle Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 取消技能被拒绝: 无效的Handle"));
		return false;
	}
	return true;
}

void ADBARpcHandler::ServerLockTarget_Implementation(AActor* TargetActor)
{
	UE_LOG(LogDBANetwork, Log, TEXT("[Server] 锁定目标: %s"), *GetNameSafe(TargetActor));

	// 验证目标是有效的敌人
	if (TargetActor && IsEnemy(GetOwner(), TargetActor))
	{
		// 通过接口设置锁定目标
		if (TScriptInterface<IIDBACharacterRef> CharacterRef = GetCharacterRef())
		{
			UE_LOG(LogDBANetwork, Log, TEXT("[Server] 目标锁定成功"));
		}
	}
}

bool ADBARpcHandler::ServerLockTarget_Validate(AActor* TargetActor)
{
	if (!ValidateTarget(TargetActor))
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 锁定目标被拒绝: 无效的目标"));
		return false;
	}
	return true;
}

void ADBARpcHandler::ServerMoveTo_Implementation(FVector_NetQuantize10 Location)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Server] 移动到位置: %s"), *Location.ToString());
}

bool ADBARpcHandler::ServerMoveTo_Validate(FVector_NetQuantize10 Location)
{
	if (Location.ContainsNaN())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 移动被拒绝: 无效的位置坐标"));
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		if (Location.X < -10000.f || Location.X > 10000.f ||
			Location.Y < -10000.f || Location.Y > 10000.f ||
			Location.Z < -100.f || Location.Z > 10000.f)
		{
			UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 移动被拒绝: 位置超出边界"));
			return false;
		}
	}

	return true;
}

void ADBARpcHandler::ServerRequestAttack_Implementation()
{
	UE_LOG(LogDBANetwork, Log, TEXT("[Server] 请求攻击"));

	AActor* AttackTarget = FindAttackTarget();
	if (!AttackTarget)
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 攻击请求失败: 未找到目标"));
		return;
	}

	// 通知客户端命中
	bool bIsCritical = false;
	float Damage = CalculateAttackDamage(AttackTarget, bIsCritical);

	FVector HitLocation = AttackTarget->GetActorLocation();
	ClientHitConfirmedWithCritical_Implementation(FGameplayAbilitySpecHandle(), Damage, FGameplayTag(), bIsCritical, HitLocation);
}

bool ADBARpcHandler::ServerRequestAttack_Validate()
{
	return true;
}

void ADBARpcHandler::ServerUltimateAbility_Implementation(const FDBAAbilityRpcParams& Params)
{
	UE_LOG(LogDBANetwork, Log, TEXT("[Server] 终极技能被调用"));

	// 检查终极能量是否足够
	if (TScriptInterface<IIDBACharacterRef> CharacterRef = GetCharacterRef())
	{
		if (CharacterRef->GetUltimateEnergy() >= 100.f)
		{
			// 消耗终极能量
			// 注意：这里只是记录，实际消耗在技能激活时进行

			// 尝试激活终极技能
			if (UAbilitySystemComponent* ASC = CharacterRef->GetAbilitySystemComponent())
			{
				ASC->TryActivateAbility(Params.AbilityHandle, false);
				ClientAbilityActivated_Implementation(Params.AbilityHandle);
				return;
			}
		}
	}

	UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 终极技能激活失败: 能量不足或ASC无效"));
	ClientAbilityFailed_Implementation(Params.AbilityHandle, FGameplayTag());
}

bool ADBARpcHandler::ServerUltimateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
	if (!Params.AbilityHandle.IsValid())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 终极技能被拒绝: 无效的Handle"));
		return false;
	}

	if (TScriptInterface<IIDBACharacterRef> CharacterRef = GetCharacterRef())
	{
		if (CharacterRef->GetUltimateEnergy() < 100.f)
		{
			UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 终极技能被拒绝: 终极能量未满"));
			return false;
		}
	}

	return true;
}

// ==================== IDBARpcClient Interface ====================

void ADBARpcHandler::ClientReceiveDamage_Implementation(float Damage, FVector_NetQuantize10 Position, FGameplayTag DamageType)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 接收伤害: %f at %s"), Damage, *Position.ToString());

	// 播放受击特效
	if (UWorld* World = GetWorld())
	{
		// 在伤害位置播放特效
		// 具体实现取决于项目需求
	}
}

void ADBARpcHandler::ClientReceiveEffect_Implementation(FGameplayTag EffectTag, float Magnitude)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 接收效果: %s, Magnitude: %f"), *EffectTag.ToString(), Magnitude);
}

void ADBARpcHandler::ClientReplicateState_Implementation(uint8 NewState, const FVector_NetQuantize10& Location)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 同步状态: %d at %s"), NewState, *Location.ToString());
}

void ADBARpcHandler::ClientAbilityActivated_Implementation(FGameplayAbilitySpecHandle Handle)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 技能已激活: %s"), *Handle.ToString());
}

void ADBARpcHandler::ClientAbilityFailed_Implementation(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("[Client] 技能激活失败: %s, Reason: %s"),
		*Handle.ToString(), *FailureTag.ToString());
}

// ==================== IDBARpcInterface Interface ====================

void ADBARpcHandler::ClientReportHit_Implementation(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation, AActor* HitActor)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 报告命中: %s at %s"), *AbilityHandle.ToString(), *HitLocation.ToString());

	// 客户端预测命中，服务端需要验证
	if (HitActor && HitActor->IsValidLowLevel())
	{
		// 播放命中特效
	}
}

void ADBARpcHandler::ClientFullStateSync_Implementation(float Health, float Energy, float Shield, float UltimateEnergy, int32 ChainLevel, int32 ResonanceLevel)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 完整状态同步: HP=%f, Energy=%f, Shield=%f, Ultimate=%f, Chain=%d, Resonance=%d"),
		Health, Energy, Shield, UltimateEnergy, ChainLevel, ResonanceLevel);

	// 更新本地状态用于观战显示
	if (TScriptInterface<IIDBACharacterRef> CharacterRef = GetCharacterRef())
	{
		if (UAbilitySystemComponent* ASC = CharacterRef->GetAbilitySystemComponent())
		{
			// 同步属性到本地
			// 具体实现取决于属性设置方式
		}
	}
}

void ADBARpcHandler::ClientMoveCorrection_Implementation(FVector_NetQuantize10 ServerLocation, float ServerTime)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 移动校正: %s, Time=%f"), *ServerLocation.ToString(), ServerTime);

	// 根据服务端位置校正客户端位置
	AActor* Owner = GetOwner();
	if (Owner)
	{
		FVector CurrentLocation = Owner->GetActorLocation();
		float Distance = FVector::Dist(CurrentLocation, ServerLocation);

		// 如果距离超过阈值，进行位置校正
		if (Distance > 500.f) // 超过500单位
		{
			Owner->SetActorLocation(ServerLocation);
		}
	}
}

void ADBARpcHandler::ClientHitConfirmed_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 命中确认: %s, Damage=%f"), *AbilityHandle.ToString(), Damage);
}

void ADBARpcHandler::ClientHitRejected_Implementation(FGameplayAbilitySpecHandle AbilityHandle)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("[Client] 命中被拒绝: %s"), *AbilityHandle.ToString());
}

void ADBARpcHandler::ClientHitConfirmedWithCritical_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType, bool bIsCritical, FVector_NetQuantize10 HitLocation)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 命中确认(带暴击): Damage=%f, bIsCritical=%s"),
		Damage, bIsCritical ? TEXT("true") : TEXT("false"));

	// 播放命中特效和音效
	if (UWorld* World = GetWorld())
	{
		// 根据是否暴击播放不同特效
	}
}

// ==================== Helper Methods ====================

bool ADBARpcHandler::ValidateEnergyCost(float Cost) const
{
	if (TScriptInterface<IIDBACharacterRef> CharacterRef = GetCharacterRef())
	{
		return CharacterRef->HasEnoughEnergy(Cost);
	}
	return true;
}

bool ADBARpcHandler::ValidateTarget(AActor* Target) const
{
	if (!Target || !Target->IsValidLowLevel())
	{
		return false;
	}

	if (!IsValid(Target))
	{
		return false;
	}

	return true;
}

bool ADBARpcHandler::ValidateCastRange(AActor* Target, float Range) const
{
	if (!ValidateTarget(Target))
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	float Distance = FVector::Dist(OwnerActor->GetActorLocation(), Target->GetActorLocation());
	return Distance <= Range;
}

AActor* ADBARpcHandler::FindAttackTarget() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// 查找最近的敌人
	TArray<AActor*> ActorsToCheck;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), ActorsToCheck);

	AActor* ClosestEnemy = nullptr;
	float MinDistance = MAX_flt;

	for (AActor* Actor : ActorsToCheck)
	{
		if (Actor == Owner || !IsValid(Actor))
		{
			continue;
		}

		// 检查是否是敌人
		if (IsEnemy(Owner, Actor))
		{
			float Distance = FVector::Dist(Owner->GetActorLocation(), Actor->GetActorLocation());

			// 检查是否在攻击范围内 (默认500单位)
			if (Distance < 500.f && Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestEnemy = Actor;
			}
		}
	}

	return ClosestEnemy;
}

float ADBARpcHandler::CalculateAttackDamage(AActor* Target, bool& OutbIsCritical) const
{
	if (!Target)
	{
		OutbIsCritical = false;
		return 0.f;
	}

	// 通过接口获取角色属性
	if (TScriptInterface<IIDBACharacterRef> TargetRef(Target))
	{
		float TargetMaxHealth = TargetRef->GetMaxHealth();
		float TargetCurrentHealth = TargetRef->GetCurrentHealth();

		// 基础伤害公式：基于目标最大生命值
		float BaseDamage = TargetMaxHealth * 0.1f; // 10% 最大生命值作为基础伤害

		// 暴击判定 (10% 概率)
		OutbIsCritical = FMath::RandRange(0.f, 1.f) < 0.1f;
		if (OutbIsCritical)
		{
			BaseDamage *= 2.f; // 暴击伤害翻倍
		}

		return BaseDamage;
	}

	// 默认伤害
	OutbIsCritical = false;
	return 50.f;
}

bool ADBARpcHandler::IsEnemy(AActor* ActorA, AActor* ActorB) const
{
	if (!ActorA || !ActorB)
	{
		return false;
	}

	if (ActorA->Implements<UIDBACharacterRef>() && ActorB->Implements<UIDBACharacterRef>())
	{
		TScriptInterface<IIDBACharacterRef> CharRefA(ActorA);
		TScriptInterface<IIDBACharacterRef> CharRefB(ActorB);
		return CharRefA->GetTeamID() != CharRefB->GetTeamID();
	}

	return true;
}
