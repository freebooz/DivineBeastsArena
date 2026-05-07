// Copyright Freebooz Games, Inc. All Rights Reserved.
// RPC Handler Implementation

#include "GameDBA/RPC/DBARpcHandler.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Engine/World.h"

ADBARpcHandler::ADBARpcHandler()
{
	SetReplicates(true);
	bAlwaysRelevant = true;
}

// ==================== IDBARpcServer Interface ====================

void ADBARpcHandler::ServerTryActivateAbility_Implementation(const FDBAAbilityRpcParams& Params)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("ServerTryActivateAbility called - handle: %s"), *Params.AbilityHandle.ToString());
}

bool ADBARpcHandler::ServerTryActivateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
	if (!Params.AbilityHandle.IsValid())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("ServerTryActivateAbility rejected: invalid handle"));
		return false;
	}

	if (Params.TargetActor != nullptr && !ValidateTarget(Params.TargetActor.Get()))
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("ServerTryActivateAbility rejected: invalid target"));
		return false;
	}

	return true;
}

void ADBARpcHandler::ServerCancelAbility_Implementation(FGameplayAbilitySpecHandle Handle)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("ServerCancelAbility called - handle: %s"), *Handle.ToString());
}

bool ADBARpcHandler::ServerCancelAbility_Validate(FGameplayAbilitySpecHandle Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("ServerCancelAbility rejected: invalid handle"));
		return false;
	}
	return true;
}

void ADBARpcHandler::ServerLockTarget_Implementation(AActor* TargetActor)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("ServerLockTarget called"));
}

bool ADBARpcHandler::ServerLockTarget_Validate(AActor* TargetActor)
{
	if (!ValidateTarget(TargetActor))
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("ServerLockTarget rejected: invalid target"));
		return false;
	}
	return true;
}

void ADBARpcHandler::ServerMoveTo_Implementation(FVector_NetQuantize10 Location)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("ServerMoveTo called - location: %s"), *Location.ToString());
}

bool ADBARpcHandler::ServerMoveTo_Validate(FVector_NetQuantize10 Location)
{
	if (Location.ContainsNaN())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("ServerMoveTo rejected: invalid location"));
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		if (Location.X < -10000.f || Location.X > 10000.f ||
			Location.Y < -10000.f || Location.Y > 10000.f ||
			Location.Z < -100.f || Location.Z > 10000.f)
		{
			UE_LOG(LogDBANetwork, Warning, TEXT("ServerMoveTo rejected: location out of bounds"));
			return false;
		}
	}

	return true;
}

void ADBARpcHandler::ServerRequestAttack_Implementation()
{
	UE_LOG(LogDBANetwork, Warning, TEXT("ServerRequestAttack called"));
}

bool ADBARpcHandler::ServerRequestAttack_Validate()
{
	return true;
}

void ADBARpcHandler::ServerUltimateAbility_Implementation(const FDBAAbilityRpcParams& Params)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("ServerUltimateAbility called"));
}

bool ADBARpcHandler::ServerUltimateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
	if (!Params.AbilityHandle.IsValid())
	{
		UE_LOG(LogDBANetwork, Warning, TEXT("ServerUltimateAbility rejected: invalid handle"));
		return false;
	}

	if (AActor* OwnerActor = GetOwner())
	{
		if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(OwnerActor))
		{
			if (Character->GetUltimateEnergy() < 100.f)
			{
				UE_LOG(LogDBANetwork, Warning, TEXT("ServerUltimateAbility rejected: ultimate energy not full"));
				return false;
			}
		}
	}

	return true;
}

// ==================== IDBARpcClient Interface ====================

void ADBARpcHandler::ClientReceiveDamage_Implementation(float Damage, FVector_NetQuantize10 Position, FGameplayTag DamageType)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("ClientReceiveDamage: %f"), Damage);
}

void ADBARpcHandler::ClientReceiveEffect_Implementation(FGameplayTag EffectTag, float Magnitude)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("ClientReceiveEffect: %s"), *EffectTag.ToString());
}

void ADBARpcHandler::ClientReplicateState_Implementation(uint8 NewState, const FVector_NetQuantize10& Location)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("ClientReplicateState: %d"), NewState);
}

void ADBARpcHandler::ClientAbilityActivated_Implementation(FGameplayAbilitySpecHandle Handle)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("ClientAbilityActivated"));
}

void ADBARpcHandler::ClientAbilityFailed_Implementation(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("ClientAbilityFailed: %s"), *FailureTag.ToString());
}

// ==================== IDBARpcInterface Interface ====================

void ADBARpcHandler::ClientReportHit_Implementation(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation, AActor* HitActor)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("ClientReportHit"));
}

void ADBARpcHandler::ClientFullStateSync_Implementation(float Health, float Energy, float Shield, float UltimateEnergy, int32 ChainLevel, int32 ResonanceLevel)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("ClientFullStateSync"));
}

void ADBARpcHandler::ClientMoveCorrection_Implementation(FVector_NetQuantize10 ServerLocation, float ServerTime)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("ClientMoveCorrection"));
}

void ADBARpcHandler::ClientHitConfirmed_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("ClientHitConfirmed"));
}

void ADBARpcHandler::ClientHitRejected_Implementation(FGameplayAbilitySpecHandle AbilityHandle)
{
	UE_LOG(LogDBANetwork, Warning, TEXT("ClientHitRejected"));
}

void ADBARpcHandler::ClientHitConfirmedWithCritical_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType, bool bIsCritical, FVector_NetQuantize10 HitLocation)
{
	UE_LOG(LogDBANetwork, Verbose, TEXT("ClientHitConfirmedWithCritical"));
}

// ==================== Helper Methods ====================

bool ADBARpcHandler::ValidateEnergyCost(float Cost) const
{
	if (AActor* OwnerActor = GetOwner())
	{
		if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(OwnerActor))
		{
			return Character->GetCurrentEnergy() >= Cost;
		}
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

AActor* ADBARpcHandler::FindAttackTarget(ADBAZodiacCharacterBase* Character) const
{
	return nullptr;
}

float ADBARpcHandler::CalculateAttackDamage(ADBAZodiacCharacterBase* Attacker, AActor* Target, bool& OutbIsCritical) const
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

	if (ADBAZodiacCharacterBase* CharA = Cast<ADBAZodiacCharacterBase>(ActorA))
	{
		if (ADBAZodiacCharacterBase* CharB = Cast<ADBAZodiacCharacterBase>(ActorB))
		{
			return CharA->GetTeamID() != CharB->GetTeamID();
		}
	}

	return true;
}
