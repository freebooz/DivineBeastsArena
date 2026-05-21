// Copyright Freebooz Games, Inc. All Rights Reserved.
// RPC 处理器实现

#include "GameDBA/RPC/DBARpcHandler.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Character/IDBACharacterRef.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"

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

// ==================== IDBARpcServer 接口 ====================

void ADBARpcHandler::ServerTryActivateAbility_Implementation(const FDBAAbilityRpcParams& Params)
{
	UE_LOG(LogDBANetwork, Log, TEXT("[服务器] 尝试激活技能，句柄=%s"), *Params.AbilityHandle.ToString());

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

	UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 激活技能失败：无法获取 ASC 或技能句柄无效。"));
	ClientAbilityFailed_Implementation(Params.AbilityHandle, FGameplayTag());
}

bool ADBARpcHandler::ServerTryActivateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
	if (!Params.AbilityHandle.IsValid())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 激活技能被拒绝：技能句柄无效。"));
		return false;
	}

	if (Params.TargetActor != nullptr && !ValidateTarget(Params.TargetActor.Get()))
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 激活技能被拒绝：目标无效。"));
		return false;
	}

	return true;
}

void ADBARpcHandler::ServerCancelAbility_Implementation(FGameplayAbilitySpecHandle Handle)
{
	UE_LOG(LogDBANetwork, Log, TEXT("[服务器] 取消技能，句柄=%s"), *Handle.ToString());

	if (TScriptInterface<IIDBACharacterRef> CharacterRef = GetCharacterRef())
	{
		if (UAbilitySystemComponent* ASC = CharacterRef->GetAbilitySystemComponent())
		{
			// 取消技能
			ASC->CancelAbilityHandle(Handle);
			return;
		}
	}

	UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 取消技能失败：无法获取 ASC。"));
}

bool ADBARpcHandler::ServerCancelAbility_Validate(FGameplayAbilitySpecHandle Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 取消技能被拒绝：技能句柄无效。"));
		return false;
	}
	return true;
}

void ADBARpcHandler::ServerLockTarget_Implementation(AActor* TargetActor)
{
	UE_LOG(LogDBANetwork, Log, TEXT("[服务器] 请求锁定目标：%s"), *GetNameSafe(TargetActor));

	// 验证目标是有效的敌人
	if (TargetActor && IsEnemy(GetOwner(), TargetActor))
	{
		// 通过接口设置锁定目标
		if (TScriptInterface<IIDBACharacterRef> CharacterRef = GetCharacterRef())
		{
			UE_LOG(LogDBANetwork, Log, TEXT("[服务器] 目标锁定成功。"));
		}
	}
}

bool ADBARpcHandler::ServerLockTarget_Validate(AActor* TargetActor)
{
	if (!ValidateTarget(TargetActor))
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 锁定目标被拒绝：目标无效。"));
		return false;
	}
	return true;
}

void ADBARpcHandler::ServerMoveTo_Implementation(FVector_NetQuantize10 Location)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[服务器] 请求移动到位置：%s"), *Location.ToString());
}

bool ADBARpcHandler::ServerMoveTo_Validate(FVector_NetQuantize10 Location)
{
	if (Location.ContainsNaN())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 移动被拒绝：位置坐标无效。"));
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		if (Location.X < DBAConstants::MapBoundary_MinX || Location.X > DBAConstants::MapBoundary_MaxX ||
			Location.Y < DBAConstants::MapBoundary_MinY || Location.Y > DBAConstants::MapBoundary_MaxY ||
			Location.Z < DBAConstants::MapBoundary_MinZ || Location.Z > DBAConstants::MapBoundary_MaxZ)
		{
			UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 移动被拒绝：位置超出边界。"));
			return false;
		}
	}

	return true;
}

void ADBARpcHandler::ServerRequestAttack_Implementation()
{
	UE_LOG(LogDBANetwork, Log, TEXT("[服务器] 收到攻击请求。"));

	AActor* AttackTarget = FindAttackTarget();
	if (!AttackTarget)
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 攻击请求失败：未找到目标。"));
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
	UE_LOG(LogDBANetwork, Log, TEXT("[服务器] 收到终极技能请求。"));

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

	UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 终极技能激活失败：能量不足或 ASC 无效。"));
	ClientAbilityFailed_Implementation(Params.AbilityHandle, FGameplayTag());
}

bool ADBARpcHandler::ServerUltimateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
	if (!Params.AbilityHandle.IsValid())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 终极技能被拒绝：技能句柄无效。"));
		return false;
	}

	if (TScriptInterface<IIDBACharacterRef> CharacterRef = GetCharacterRef())
	{
		if (CharacterRef->GetUltimateEnergy() < 100.f)
		{
			UE_LOG(LogDBANetwork, Warning, TEXT("[服务器] 终极技能被拒绝：终极能量未满。"));
			return false;
		}
	}

	return true;
}

// ==================== IDBARpcClient 接口 ====================

void ADBARpcHandler::ClientReceiveDamage_Implementation(float Damage, FVector_NetQuantize10 Position, FGameplayTag DamageType)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[客户端] 接收伤害：伤害=%f 位置=%s"), Damage, *Position.ToString());

	// 播放受击特效
	if (UWorld* World = GetWorld())
	{
		// 在伤害位置播放特效
		// 具体实现取决于项目需求
	}
}

void ADBARpcHandler::ClientReceiveEffect_Implementation(FGameplayTag EffectTag, float Magnitude)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[客户端] 接收效果：效果=%s 强度=%f"), *EffectTag.ToString(), Magnitude);
}

void ADBARpcHandler::ClientReplicateState_Implementation(uint8 NewState, const FVector_NetQuantize10& Location)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[客户端] 同步状态：状态=%d 位置=%s"), NewState, *Location.ToString());
}

void ADBARpcHandler::ClientAbilityActivated_Implementation(FGameplayAbilitySpecHandle Handle)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[客户端] 技能已激活：%s"), *Handle.ToString());
}

void ADBARpcHandler::ClientAbilityFailed_Implementation(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("[客户端] 技能激活失败：句柄=%s 原因=%s"),
		*Handle.ToString(), *FailureTag.ToString());
}

// ==================== IDBARpcInterface 接口 ====================

void ADBARpcHandler::ClientReportHit_Implementation(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation, AActor* HitActor)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[客户端] 上报命中：技能句柄=%s 位置=%s"), *AbilityHandle.ToString(), *HitLocation.ToString());

	// 客户端预测命中，服务端需要验证
	if (HitActor && IsValid(HitActor))
	{
		// 播放命中特效
	}
}

void ADBARpcHandler::ClientFullStateSync_Implementation(float Health, float Energy, float Shield, float UltimateEnergy, int32 ChainLevel, int32 ResonanceLevel)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[客户端] 完整状态同步：生命=%f 能量=%f 护盾=%f 终极能量=%f 连锁=%d 共鸣=%d"),
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
	UE_LOG(LogDBANetwork, Verbose, TEXT("[客户端] 移动校正：服务器位置=%s 时间=%f"), *ServerLocation.ToString(), ServerTime);

	// 根据服务端位置校正客户端位置
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		FVector CurrentLocation = OwnerActor->GetActorLocation();
		float Distance = FVector::Dist(CurrentLocation, ServerLocation);

		// 如果距离超过阈值，进行位置校正
		if (Distance > DBAConstants::MapBoundary_CorrectionThreshold)
		{
			Owner->SetActorLocation(ServerLocation);
		}
	}
}

void ADBARpcHandler::ClientHitConfirmed_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[客户端] 命中确认：技能句柄=%s 伤害=%f"), *AbilityHandle.ToString(), Damage);
}

void ADBARpcHandler::ClientHitRejected_Implementation(FGameplayAbilitySpecHandle AbilityHandle)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("[客户端] 命中被拒绝：%s"), *AbilityHandle.ToString());
}

void ADBARpcHandler::ClientHitConfirmedWithCritical_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType, bool bIsCritical, FVector_NetQuantize10 HitLocation)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[客户端] 命中确认（含暴击）：伤害=%f 是否暴击=%s"),
		Damage, bIsCritical ? TEXT("是") : TEXT("否"));

	// 播放命中特效和音效
	if (UWorld* World = GetWorld())
	{
		// 根据是否暴击播放不同特效
	}
}

// ==================== 辅助方法 ====================

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
	if (!Target || !IsValid(Target))
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
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const float AttackRange = DBAConstants::DefaultAttackRange;

	// 使用球体Overlap查询代替遍历所有Actor
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	FCollisionShape SphereShape;
	SphereShape.SetSphere(AttackRange);

	if (UWorld* World = GetWorld())
	{
		World->OverlapMultiByObjectType(
			Overlaps,
			OwnerLocation,
			FQuat::Identity,
			FCollisionObjectQueryParams(ECC_Pawn),
			SphereShape,
			QueryParams
		);

		AActor* ClosestEnemy = nullptr;
		float MinDistance = MAX_flt;

		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* OtherActor = Overlap.GetActor();
			if (!OtherActor || !IsValid(OtherActor))
			{
				continue;
			}

			// 检查是否是敌人
			if (IsEnemy(Owner, OtherActor))
			{
				float Distance = FVector::Dist(OwnerLocation, OtherActor->GetActorLocation());
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
					ClosestEnemy = OtherActor;
				}
			}
		}

		return ClosestEnemy;
	}

	return nullptr;
}

float ADBARpcHandler::CalculateAttackDamage(AActor* Target, bool& OutbIsCritical) const
{
	if (!Target)
	{
		OutbIsCritical = false;
		return 0.f;
	}

	// 通过接口获取角色属性
	IIDBACharacterRef* TargetRefInterface = Cast<IIDBACharacterRef>(Target);
	if (TargetRefInterface)
	{
		float TargetMaxHealth = TargetRefInterface->GetMaxHealth();
		float TargetCurrentHealth = TargetRefInterface->GetCurrentHealth();

		// 基础伤害公式：基于目标最大生命值
		float BaseDamage = TargetMaxHealth * DBAConstants::BaseDamagePercentOfMaxHealth;

		// 暴击判定
		OutbIsCritical = FMath::RandRange(0.f, 1.f) < DBAConstants::CriticalChance;
		if (OutbIsCritical)
		{
			BaseDamage *= DBAConstants::CriticalDamageMultiplier;
		}

		return BaseDamage;
	}

	// 默认伤害
	OutbIsCritical = false;
	return DBAConstants::DefaultBaseDamage;
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

	// Actor未实现角色接口，既不是敌人也不是友军（无效目标）
	return false;
}
