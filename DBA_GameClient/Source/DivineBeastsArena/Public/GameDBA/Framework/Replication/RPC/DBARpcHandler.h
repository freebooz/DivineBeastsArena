// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/Networking/RPC/DBARpcClient.h"
#include "GameMoba/Networking/RPC/DBARpcServer.h"
#include "GameDBA/Characters/IDBACharacterRef.h"
#include "DBARpcHandler.generated.h"

UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API ADBARpcHandler : public AActor, public IDBARpcServer, public IDBARpcClient
{
	GENERATED_BODY()

public:
	ADBARpcHandler();

	virtual void ServerTryActivateAbility(const FDBAAbilityRpcParams& Params) override
	{
		if (ServerTryActivateAbility_Validate(Params))
		{
			ServerTryActivateAbility_Implementation(Params);
		}
	}
	virtual void ServerCancelAbility(FGameplayAbilitySpecHandle Handle) override
	{
		if (ServerCancelAbility_Validate(Handle))
		{
			ServerCancelAbility_Implementation(Handle);
		}
	}
	virtual void ServerLockTarget(AActor* TargetActor) override
	{
		if (ServerLockTarget_Validate(TargetActor))
		{
			ServerLockTarget_Implementation(TargetActor);
		}
	}
	virtual void ServerMoveTo(FVector_NetQuantize10 Location) override
	{
		if (ServerMoveTo_Validate(Location))
		{
			ServerMoveTo_Implementation(Location);
		}
	}
	virtual void ServerRequestAttack() override
	{
		if (ServerRequestAttack_Validate())
		{
			ServerRequestAttack_Implementation();
		}
	}
	virtual void ServerUltimateAbility(const FDBAAbilityRpcParams& Params) override
	{
		if (ServerUltimateAbility_Validate(Params))
		{
			ServerUltimateAbility_Implementation(Params);
		}
	}
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

	UFUNCTION(BlueprintCallable, Category = "DBA|RPC|Target")
	AActor* GetLockedTargetActor() const { return LockedTargetActor.Get(); }

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
	bool ValidateServerCharacterContext(const TCHAR* OperationName) const;
	bool ValidateEnergyCost(float Cost) const;
	bool ValidateAbilityInputSemantics(const FDBAAbilityRpcParams& Params, bool bRequireUltimate) const;
	bool ValidateAbilityCooldown(const FDBAAbilityRpcParams& Params) const;
	bool ValidateTarget(AActor* Target) const;
	bool ValidateCastRange(AActor* Target, float Range) const;
	AActor* FindAttackTarget();
	float CalculateAttackDamage(AActor* Target, bool& OutbIsCritical) const;
	bool IsEnemy(AActor* ActorA, AActor* ActorB) const;

	/** 通过接口获取 Owner Character */
	TScriptInterface<IIDBACharacterRef> GetCharacterRef() const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> LockedTargetActor = nullptr;

	/** 上次攻击请求的服务器时间戳，用于频率校验 */
	float LastAttackRequestTime = -1.0f;
};
