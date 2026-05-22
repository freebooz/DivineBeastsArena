// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Combat/DBAClientPredictionComponent.h"

#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/RPC/DBARpcHandler.h"

UDBAClientPredictionComponent::UDBAClientPredictionComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UDBAClientPredictionComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UDBAClientPredictionComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
}

void UDBAClientPredictionComponent::TryPredictAbility(FName SkillId, AActor* Target, FVector TargetLocation)
{
	if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner()))
	{
		if (ADBARpcHandler* RpcHandler = Character->GetRpcHandler())
		{
			FDBAAbilityRpcParams Params;
			Params.AbilityHandle = FGameplayAbilitySpecHandle();
			Params.TargetActor = Target;
			Params.TargetLocation = TargetLocation;
			RpcHandler->ServerTryActivateAbility(Params);
		}
	}
}

void UDBAClientPredictionComponent::TryPredictMove(FVector TargetLocation)
{
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
	LastCorrectedLocation = ServerLocation;
	PredictionError = FVector::Dist(PredictedLocation, ServerLocation);
	OnMoveCorrected(ServerLocation);
}

void UDBAClientPredictionComponent::OnAbilityActivated(FGameplayAbilitySpecHandle Handle, bool bSuccess)
{
}

void UDBAClientPredictionComponent::OnMoveCorrected(FVector CorrectedLocation)
{
	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorLocation(CorrectedLocation);
	}
}