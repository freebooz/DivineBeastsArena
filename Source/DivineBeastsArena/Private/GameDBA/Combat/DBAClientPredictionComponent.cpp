// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/DBAClientPredictionComponent.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameMoba/RPC/DBARpcInterface.h"

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
		if (IDBARpcServer* RpcServer = Cast<IDBARpcServer>(Character))
		{
			FDBAAbilityRpcParams Params;
			Params.AbilityHandle = FGameplayAbilitySpecHandle();
			Params.TargetActor = Target;
			Params.TargetLocation = TargetLocation;

			RpcServer->ServerTryActivateAbility(Params);
		}
	}
}

void UDBAClientPredictionComponent::TryPredictMove(FVector TargetLocation)
{
	if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner()))
	{
		if (IDBARpcServer* RpcServer = Cast<IDBARpcServer>(Character))
		{
			PredictedLocation = TargetLocation;
			RpcServer->ServerMoveTo(TargetLocation);
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
	// 处理技能激活结果
}

void UDBAClientPredictionComponent::OnMoveCorrected(FVector CorrectedLocation)
{
	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorLocation(CorrectedLocation);
	}
}