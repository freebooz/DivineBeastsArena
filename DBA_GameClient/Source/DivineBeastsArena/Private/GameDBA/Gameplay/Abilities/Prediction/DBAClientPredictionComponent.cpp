// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Gameplay/Abilities/Prediction/DBAClientPredictionComponent.h"

#include "Engine/World.h"
#include "GameDBA/Characters/DBAZodiacCharacterBase.h"
#include "GameDBA/Gameplay/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/Framework/Replication/RPC/DBARpcHandler.h"

namespace
{
	int32 ResolvePredictionAbilityInputID(FName SkillId)
	{
		if (SkillId == TEXT("Skill01") || SkillId == TEXT("Skill1"))
		{
			return static_cast<int32>(EDBAAbilityInputID::Skill01);
		}
		if (SkillId == TEXT("Skill02") || SkillId == TEXT("Skill2"))
		{
			return static_cast<int32>(EDBAAbilityInputID::Skill02);
		}
		if (SkillId == TEXT("Skill03") || SkillId == TEXT("Skill3"))
		{
			return static_cast<int32>(EDBAAbilityInputID::Skill03);
		}
		if (SkillId == TEXT("Skill04") || SkillId == TEXT("Skill4"))
		{
			return static_cast<int32>(EDBAAbilityInputID::Skill04);
		}
		if (SkillId == TEXT("Ultimate") || SkillId == TEXT("Skill05") || SkillId == TEXT("Skill5"))
		{
			return static_cast<int32>(EDBAAbilityInputID::Ultimate);
		}

		return static_cast<int32>(EDBAAbilityInputID::None);
	}
}

UDBAClientPredictionComponent::UDBAClientPredictionComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UDBAClientPredictionComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UDBAClientPredictionComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
}

bool UDBAClientPredictionComponent::IsPredictionRuntimeAllowed() const
{
	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return false;
	}

	const ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner());
	return Character && Character->IsLocallyControlled();
}

void UDBAClientPredictionComponent::TryPredictAbility(FName SkillId, AActor* Target, FVector TargetLocation)
{
	if (!IsPredictionRuntimeAllowed())
	{
		return;
	}

	const int32 AbilityInputID = ResolvePredictionAbilityInputID(SkillId);
	if (AbilityInputID == static_cast<int32>(EDBAAbilityInputID::None))
	{
		return;
	}

	if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner()))
	{
		UDBAAbilitySystemComponent* ASC = Character->GetDBAAbilitySystemComponent();
		if (!ASC)
		{
			return;
		}

		const FGameplayAbilitySpecHandle AbilityHandle = ASC->FindAbilitySpecHandleByInputID(AbilityInputID);
		if (!AbilityHandle.IsValid())
		{
			return;
		}

		if (ADBARpcHandler* RpcHandler = Character->GetRpcHandler())
		{
			FDBAAbilityRpcParams Params;
			Params.AbilityHandle = AbilityHandle;
			Params.TargetActor = Target;
			Params.TargetLocation = TargetLocation;
			if (AbilityInputID == static_cast<int32>(EDBAAbilityInputID::Ultimate))
			{
				RpcHandler->ServerUltimateAbility(Params);
			}
			else
			{
				RpcHandler->ServerTryActivateAbility(Params);
			}
		}
	}
}

void UDBAClientPredictionComponent::TryPredictMove(FVector TargetLocation)
{
	if (!IsPredictionRuntimeAllowed())
	{
		return;
	}

	if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner()))
	{
		if (ADBARpcHandler* RpcHandler = Character->GetRpcHandler())
		{
			PredictedLocation = TargetLocation;
			RpcHandler->ServerMoveTo(FVector_NetQuantize10(TargetLocation));
		}
	}
}

void UDBAClientPredictionComponent::ApplyServerCorrection(FVector ServerLocation, float ServerTime)
{
	static_cast<void>(ServerTime);

	if (!IsPredictionRuntimeAllowed())
	{
		return;
	}

	LastCorrectedLocation = ServerLocation;
	PredictionError = FVector::Dist(PredictedLocation, ServerLocation);
	OnMoveCorrected(ServerLocation);
}

void UDBAClientPredictionComponent::OnAbilityActivated(FGameplayAbilitySpecHandle Handle, bool bSuccess)
{
}

void UDBAClientPredictionComponent::OnMoveCorrected(FVector CorrectedLocation)
{
	if (!IsPredictionRuntimeAllowed())
	{
		return;
	}

	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorLocation(CorrectedLocation);
	}
}
