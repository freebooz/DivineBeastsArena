// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameMoba - 通用MOBA AbilitySystemComponent基类

#include "GameMoba/GAS/DBAMobaAbilitySystemComponentBase.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

UDBAMobaAbilitySystemComponentBase::UDBAMobaAbilitySystemComponentBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAbilitiesInitialized = false;
}

void UDBAMobaAbilitySystemComponentBase::BeginPlay()
{
	Super::BeginPlay();
}

void UDBAMobaAbilitySystemComponentBase::InitializeAbilities(AActor* InOwnerActor, AActor* InAvatarActor)
{
	if (!InOwnerActor || !InAvatarActor)
	{
		return;
	}

	InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	bAbilitiesInitialized = true;
}

void UDBAMobaAbilitySystemComponentBase::RemoveAllGrantedAbilities()
{
	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		ClearAbility(Handle);
	}
	GrantedAbilityHandles.Empty();
}
