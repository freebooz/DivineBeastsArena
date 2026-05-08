// Copyright Freebooz Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameMoba/RPC/DBARpcClient.h"
#include "GameMoba/RPC/DBARpcServer.h"
#include "GameDBA/Character/IDBACharacterRef.h"
#include "DBARpcHandler.generated.h"

UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API ADBARpcHandler : public AActor, public IDBARpcServer, public IDBARpcClient
{
	GENERATED_BODY()

public:
	ADBARpcHandler();

		virtual void ServerTryActivateAbility(const FDBAAbilityRpcParams& Params) override { ServerTryActivateAbility_Implementation(Params); }
	virtual void ServerCancelAbility(FGameplayAbilitySpecHandle Handle) override { ServerCancelAbility_Implementation(Handle); }
	virtual void ServerLockTarget(AActor* TargetActor) override { ServerLockTarget_Implementation(TargetActor); }
	virtual void ServerMoveTo(FVector_NetQuantize10 Location) override { ServerMoveTo_Implementation(Location); }
	virtual void ServerRequestAttack() override { ServerRequestAttack_Implementation(); }
	virtual void ServerUltimateAbility(const FDBAAbilityRpcParams& Params) override { ServerUltimateAbility_Implementation(Params); }
	virtual void ClientReceiveDamage(float Damage, FVector_NetQuantize10 Position, FGameplayTag DamageType) override { ClientReceiveDamage_Implementation(Damage, Position, DamageType); }
	virtual void ClientReceiveEffect(FGameplayTag EffectTag, float Magnitude) override { ClientReceiveEffect_Implementation(EffectTag, Magnitude); }
	virtual void ClientReplicateState(uint8 NewState, const FVector_NetQuantize10& Location) override { ClientReplicateState_Implementation(NewState, Location); }
	virtual void ClientAbilityActivated(FGameplayAbilitySpecHandle Handle) override { ClientAbilityActivated_Implementation(Handle); }
	virtual void ClientAbilityFailed(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag) override { ClientAbilityFailed_Implementation(Handle, FailureTag); }
	virtual void ClientReportHit(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation, AActor* HitActor) override { ClientReportHit_Implementation(AbilityHandle, HitLocation, HitActor); }
	virtual void ClientFullStateSync(float Health, float Energy, float Shield, float UltimateEnergy, int32 ChainLevel, int32 ResonanceLevel) override { ClientFullStateSync_Implementation(Health, Energy, Shield, UltimateEnergy, ChainLevel, ResonanceLevel); }
	virtual void ClientMoveCorrection(FVector_NetQuantize10 ServerLocation, float ServerTime) override { ClientMoveCorrection_Implementation(ServerLocation, ServerTime); }
	virtual void ClientHitConfirmed(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType) override { ClientHitConfirmed_Implementation(AbilityHandle, Damage, DamageType); }
	virtual void ClientHitRejected(FGameplayAbilitySpecHandle AbilityHandle) override { ClientHitRejected_Implementation(AbilityHandle); }
	virtual void ClientHitConfirmedWithCritical(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType, bool bIsCritical, FVector_NetQuantize10 HitLocation) override { ClientHitConfirmedWithCritical_Implementation(AbilityHandle, Damage, DamageType, bIsCritical, HitLocation); }
	// ==================== IDBARpcServer Interface ====================
	virtual void ServerTryActivateAbility_Implementation(const FDBAAbilityRpcParams& Params) override;
	virtual bool ServerTryActivateAbility_Validate(const FDBAAbilityRpcParams& Params) override;

	virtual void ServerCancelAbility_Implementation(FGameplayAbilitySpecHandle Handle) override;
	virtual bool ServerCancelAbility_Validate(FGameplayAbilitySpecHandle Handle) override;

	virtual void ServerLockTarget_Implementation(AActor* TargetActor) override;
	virtual bool ServerLockTarget_Validate(AActor* TargetActor) override;

	virtual void ServerMoveTo_Implementation(FVector_NetQuantize10 Location) override;
	virtual bool ServerMoveTo_Validate(FVector_NetQuantize10 Location) override;

	virtual void ServerRequestAttack_Implementation() override;
	virtual bool ServerRequestAttack_Validate() override;

	virtual void ServerUltimateAbility_Implementation(const FDBAAbilityRpcParams& Params) override;
	virtual bool ServerUltimateAbility_Validate(const FDBAAbilityRpcParams& Params) override;

	// ==================== IDBARpcClient Interface ====================
	virtual void ClientReceiveDamage_Implementation(float Damage, FVector_NetQuantize10 Position, FGameplayTag DamageType) override;
	virtual void ClientReceiveEffect_Implementation(FGameplayTag EffectTag, float Magnitude) override;
	virtual void ClientReplicateState_Implementation(uint8 NewState, const FVector_NetQuantize10& Location) override;
	virtual void ClientAbilityActivated_Implementation(FGameplayAbilitySpecHandle Handle) override;
	virtual void ClientAbilityFailed_Implementation(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag) override;

	// ==================== IDBARpcInterface Interface ====================
	virtual void ClientReportHit_Implementation(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation, AActor* HitActor) override;
	virtual void ClientFullStateSync_Implementation(float Health, float Energy, float Shield, float UltimateEnergy, int32 ChainLevel, int32 ResonanceLevel) override;
	virtual void ClientMoveCorrection_Implementation(FVector_NetQuantize10 ServerLocation, float ServerTime) override;
	virtual void ClientHitConfirmed_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType) override;
	virtual void ClientHitRejected_Implementation(FGameplayAbilitySpecHandle AbilityHandle) override;
	virtual void ClientHitConfirmedWithCritical_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType, bool bIsCritical, FVector_NetQuantize10 HitLocation) override;

protected:
	bool ValidateEnergyCost(float Cost) const;
	bool ValidateTarget(AActor* Target) const;
	bool ValidateCastRange(AActor* Target, float Range) const;
	AActor* FindAttackTarget() const;
	float CalculateAttackDamage(AActor* Target, bool& OutbIsCritical) const;
	bool IsEnemy(AActor* ActorA, AActor* ActorB) const;

	/** 通过接口获取 Owner Character */
	TScriptInterface<IIDBACharacterRef> GetCharacterRef() const;
};
