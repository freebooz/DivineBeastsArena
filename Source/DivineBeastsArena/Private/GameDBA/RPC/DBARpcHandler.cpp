// Copyright Freebooz Games, Inc. All Rights Reserved.
// RPC Handler Implementation

#include "GameDBA/RPC/DBARpcHandler.h"
#include "GameDBA/Core/DBALogChannels.h"
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
	UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 尝试激活技能 - Handle: %s"), *Params.AbilityHandle.ToString());
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
	UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 取消技能 - Handle: %s"), *Handle.ToString());
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
	UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 锁定目标"));
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
	UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 请求攻击"));
}

bool ADBARpcHandler::ServerRequestAttack_Validate()
{
	return true;
}

void ADBARpcHandler::ServerUltimateAbility_Implementation(const FDBAAbilityRpcParams& Params)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("[Server] 终极技能被调用"));
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
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 接收伤害: %f"), Damage);
}

void ADBARpcHandler::ClientReceiveEffect_Implementation(FGameplayTag EffectTag, float Magnitude)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 接收效果: %s"), *EffectTag.ToString());
}

void ADBARpcHandler::ClientReplicateState_Implementation(uint8 NewState, const FVector_NetQuantize10& Location)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 同步状态: %d"), NewState);
}

void ADBARpcHandler::ClientAbilityActivated_Implementation(FGameplayAbilitySpecHandle Handle)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 技能已激活"));
}

void ADBARpcHandler::ClientAbilityFailed_Implementation(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("[Client] 技能激活失败: %s"), *FailureTag.ToString());
}

// ==================== IDBARpcInterface Interface ====================

void ADBARpcHandler::ClientReportHit_Implementation(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation, AActor* HitActor)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 报告命中"));
}

void ADBARpcHandler::ClientFullStateSync_Implementation(float Health, float Energy, float Shield, float UltimateEnergy, int32 ChainLevel, int32 ResonanceLevel)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 完整状态同步"));
}

void ADBARpcHandler::ClientMoveCorrection_Implementation(FVector_NetQuantize10 ServerLocation, float ServerTime)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 移动校正"));
}

void ADBARpcHandler::ClientHitConfirmed_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 命中确认"));
}

void ADBARpcHandler::ClientHitRejected_Implementation(FGameplayAbilitySpecHandle AbilityHandle)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("[Client] 命中被拒绝"));
}

void ADBARpcHandler::ClientHitConfirmedWithCritical_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType, bool bIsCritical, FVector_NetQuantize10 HitLocation)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("[Client] 命中确认(带暴击)"));
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
	return nullptr;
}

float ADBARpcHandler::CalculateAttackDamage(AActor* Target, bool& OutbIsCritical) const
{
	OutbIsCritical = false;
	return 100.f;
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
