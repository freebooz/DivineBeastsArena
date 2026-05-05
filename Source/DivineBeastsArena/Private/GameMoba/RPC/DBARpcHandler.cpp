// Copyright Freebooz Games, Inc. All Rights Reserved.
#include "GameMoba/RPC/DBARpcHandler.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Combat/DBADamageCalculator.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AbilitySystemComponent.h"

ADBARpcHandler::ADBARpcHandler()
{
    bReplicates = true;
    NetUpdateFrequency = 30.0f;
}

bool ADBARpcHandler::ServerTryActivateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
    // 基础验证 - 可以在子类重写
    if (!Params.AbilityHandle.IsValid()) return false;
    return true;
}

void ADBARpcHandler::ServerTryActivateAbility_Implementation(const FDBAAbilityRpcParams& Params)
{
    if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner()))
    {
        if (UDBAAbilitySystemComponent* ASC = Character->GetDBAAbilitySystemComponent())
        {
            ASC->TryActivateAbility(Params.AbilityHandle);
        }
    }
}

bool ADBARpcHandler::ServerCancelAbility_Validate(FGameplayAbilitySpecHandle Handle)
{
    return Handle.IsValid();
}

void ADBARpcHandler::ServerCancelAbility_Implementation(FGameplayAbilitySpecHandle Handle)
{
    if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner()))
    {
        if (UDBAAbilitySystemComponent* ASC = Character->GetDBAAbilitySystemComponent())
        {
            ASC->CancelAbilitySpec(Handle);
        }
    }
}

bool ADBARpcHandler::ServerLockTarget_Validate(AActor* TargetActor)
{
    return ValidateTarget(TargetActor);
}

void ADBARpcHandler::ServerLockTarget_Implementation(AActor* TargetActor)
{
    // 实现目标锁定逻辑
}

bool ADBARpcHandler::ServerMoveTo_Validate(FVector_NetQuantize10 Location)
{
    // 简单验证位置有效性
    return !Location.IsZero();
}

void ADBARpcHandler::ServerMoveTo_Implementation(FVector_NetQuantize10 Location)
{
	if (!HasAuthority())
	{
		return; // 仅服务端执行
	}

	ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner());
	if (!Character)
	{
		return;
	}

	// 使用 CharacterMovement 进行导航
	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);

		// 使用 NavigationSystem 查询路径
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys)
		{
			FNavLocation NavLocation;
			// 确保目标点在导航网格上
			if (NavSys->GetRandomPointInNavigableRadius(Location, 50.0f, NavLocation))
			{
				// 使用 RequestDirectMove 沿路径移动
				Movement->RequestDirectMove(NavLocation.Location, true);
			}
			else
			{
				// 如果无法找到路径，直接设置目标
				Movement->RequestDirectMove(Location, true);
			}
		}
	}

	// 检查位置偏差，决定是否需要校正
	float Distance = FVector::Dist(Character->GetActorLocation(), FVector(Location));
	if (Distance > 50.0f) // 校正阈值
	{
		// 发送位置校正到客户端
		if (IDBARpcClient* ClientInterface = Cast<IDBARpcClient>(this))
		{
			ClientInterface->ClientMoveCorrection(
				Character->GetActorLocation(),
				GetWorld()->GetTimeSeconds());
		}
	}
}

bool ADBARpcHandler::ServerRequestAttack_Validate()
{
    return true; // 普攻无特殊验证
}

void ADBARpcHandler::ServerRequestAttack_Implementation()
{
	if (!HasAuthority())
	{
		return; // 仅服务端执行
	}

	// 获取拥有者（角色）
	ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner());
	if (!Character)
	{
		return;
	}

	// 获取目标
	AActor* Target = FindAttackTarget(Character);
	if (!Target)
	{
		return; // 无有效目标
	}

	// 计算伤害
	bool bIsCritical = false;
	float FinalDamage = CalculateAttackDamage(Character, Target, bIsCritical);

	// 应用伤害
	UDBADamageCalculator::ApplyDamageToTarget(Character, Target, FinalDamage,
		Character->ElementType, bIsCritical);

	// 通知客户端命中结果
	if (IDBARpcClient* ClientInterface = Cast<IDBARpcClient>(this))
	{
		FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Damage.Physical"));
		ClientInterface->ClientHitConfirmedWithCritical(
			FGameplayAbilitySpecHandle(),
			FinalDamage,
			DamageTag,
			bIsCritical,
			Target->GetActorLocation());
	}
}

AActor* ADBARpcHandler::FindAttackTarget(ADBAZodiacCharacterBase* Character) const
{
	if (!Character)
	{
		return nullptr;
	}

	// 简单实现：获取范围内最近的敌方角色
	// 实际项目中应该使用目标锁定组件
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetOverlappingActors(Character, FoundActors, AActor::StaticClass());

	AActor* NearestTarget = nullptr;
	float MinDistance = FLT_MAX;

	for (AActor* Actor : FoundActors)
	{
		// 检查是否是敌方
		if (IsEnemy(Character, Actor))
		{
			float Distance = Character->GetDistanceTo(Actor);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				NearestTarget = Actor;
			}
		}
	}

	return NearestTarget;
}

float ADBARpcHandler::CalculateAttackDamage(ADBAZodiacCharacterBase* Attacker, AActor* Target, bool& OutbIsCritical) const
{
	if (!Attacker || !Target)
	{
		OutbIsCritical = false;
		return 0.0f;
	}

	// 获取攻击者和防御者的属性
	float AttackPower = 100.0f; // 默认攻击力
	float Defense = 50.0f; // 默认防御力
	float CriticalRate = 0.1f;
	float CriticalMultiplier = 2.0f;

	// 从ASC获取实际属性值
	if (UAbilitySystemComponent* AttackerASC = Attacker->FindComponentByClass<UAbilitySystemComponent>())
	{
		if (UDBABattleAttributeSet* AttackerAttr = Cast<UDBABattleAttributeSet>(AttackerASC->GetAttributeSet(UDBABattleAttributeSet::StaticClass())))
		{
			AttackPower = AttackerAttr->GetAttackPower();
			CriticalRate = AttackerAttr->GetCriticalRate();
			CriticalMultiplier = AttackerAttr->GetCriticalMultiplier();
		}
	}

	if (UAbilitySystemComponent* TargetASC = Target->FindComponentByClass<UAbilitySystemComponent>())
	{
		if (UDBABattleAttributeSet* TargetAttr = Cast<UDBABattleAttributeSet>(TargetASC->GetAttributeSet(UDBABattleAttributeSet::StaticClass())))
		{
			Defense = TargetAttr->GetDefense();
		}
	}

	// 使用伤害计算器计算最终伤害
	return UDBADamageCalculator::CalculateFinalDamage(
		AttackPower,
		EDBAElement::None, // 普通攻击无元素
		EDBAElement::None,
		Attacker->GetResonanceLevel(),
		Attacker->GetChainLevel(),
		Defense,
		CriticalRate,
		CriticalMultiplier,
		OutbIsCritical);
}

bool ADBARpcHandler::IsEnemy(AActor* ActorA, AActor* ActorB) const
{
	// 简化实现：检查是否是不同阵营
	// 实际项目中应该有阵营系统
	return ActorA != ActorB;
}

bool ADBARpcHandler::ServerUltimateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
    // 大招验证更严格
    if (!Params.AbilityHandle.IsValid()) return false;
    return ValidateEnergyCost(100.0f); // 大招需要能量
}

void ADBARpcHandler::ServerUltimateAbility_Implementation(const FDBAAbilityRpcParams& Params)
{
    ServerTryActivateAbility_Implementation(Params);
}

bool ADBARpcHandler::ValidateEnergyCost(float Cost) const
{
    return true; // 简化实现
}

bool ADBARpcHandler::ValidateTarget(AActor* Target) const
{
    return Target != nullptr && IsValid(Target);
}

bool ADBARpcHandler::ValidateCastRange(AActor* Target, float Range) const
{
    if (!Target) return false;
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= Range;
}

void ADBARpcHandler::ClientReceiveDamage_Implementation(float Damage, FVector_NetQuantize10 Position, FGameplayTag DamageType)
{
    // 客户端显示伤害
}

void ADBARpcHandler::ClientReceiveEffect_Implementation(FGameplayTag EffectTag, float Magnitude)
{
    // 客户端播放效果
}

void ADBARpcHandler::ClientReplicateState_Implementation(uint8 NewState, const FVector_NetQuantize10& Location)
{
    // 客户端同步状态
}

void ADBARpcHandler::ClientAbilityActivated_Implementation(FGameplayAbilitySpecHandle Handle)
{
    // 客户端显示技能激活
}

void ADBARpcHandler::ClientAbilityFailed_Implementation(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag)
{
    // 客户端显示技能失败
}

void ADBARpcHandler::ClientReportHit_Implementation(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation, AActor* HitActor)
{
    // 服务端收到客户端报告的命中，开始验证
}

void ADBARpcHandler::ClientFullStateSync_Implementation(float Health, float Energy, float Shield, float UltimateEnergy, int32 ChainLevel, int32 ResonanceLevel)
{
    // 客户端收到完整状态同步，用于断线重连后恢复状态
}

void ADBARpcHandler::ClientMoveCorrection_Implementation(FVector_NetQuantize10 ServerLocation, float ServerTime)
{
	// 客户端收到服务端位置校正，平滑移动到正确位置
	ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner());
	if (!Character)
	{
		return;
	}

	// 停止当前移动
	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->Velocity = FVector::ZeroVector;
		Movement->SetMovementMode(MOVE_Walking);
	}

	// 平滑插值到服务端位置（延迟补偿）
	// 在实际实现中应该使用 Timeline 或 Tick 配合插值
	Character->SetActorLocation(FVector(ServerLocation));
}

void ADBARpcHandler::ClientHitConfirmed_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType)
{
    // 服务端确认命中，客户端播放命中特效
}

void ADBARpcHandler::ClientHitRejected_Implementation(FGameplayAbilitySpecHandle AbilityHandle)
{
    // 服务端拒绝命中，客户端撤销之前预判的命中效果
}

void ADBARpcHandler::ClientHitConfirmedWithCritical_Implementation(
    FGameplayAbilitySpecHandle AbilityHandle,
    float Damage,
    FGameplayTag DamageType,
    bool bIsCritical,
    FVector_NetQuantize10 HitLocation)
{
    // 服务端确认命中，客户端播放命中特效
    // 如果是暴击，播放暴击特效
    if (bIsCritical)
    {
        // TODO: 播放暴击特效和伤害数字
    }
    else
    {
        // TODO: 播放普通命中特效
    }
}