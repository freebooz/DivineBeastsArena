// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GameInstance/DBAGameInstance.h"

#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const FName FrontendMapPath(TEXT("/Game/Maps/Lobby/FrontendMap"));

	bool IsFrontendWorld(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("FrontendMap"));
	}
}

UDBAGameInstance::UDBAGameInstance()
{
}

void UDBAGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] Initialized."));
}

void UDBAGameInstance::Shutdown()
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] Shutdown."));
	Super::Shutdown();
}

void UDBAGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	Super::OnWorldChanged(OldWorld, NewWorld);

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] World changed: %s -> %s"),
		OldWorld ? *OldWorld->GetName() : TEXT("None"),
		NewWorld ? *NewWorld->GetName() : TEXT("None"));

	if (bPendingStartLoginFlowOnFrontend && IsFrontendWorld(NewWorld))
	{
		bPendingStartLoginFlowOnFrontend = false;
		StartLoginFlow();
	}
}

void UDBAGameInstance::StartLoginFlow()
{
	if (UWorld* World = GetWorld())
	{
		if (!IsFrontendWorld(World))
		{
			bPendingStartLoginFlowOnFrontend = true;
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] Switching to FrontendMap before login flow."));
			UGameplayStatics::OpenLevel(World, FrontendMapPath);
			return;
		}
	}

	if (bLoginFlowStarted)
	{
		return;
	}

	bLoginFlowStarted = true;
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] Start login flow."));

	if (UDBALoginFlowSubsystem* LoginFlow = GetSubsystem<UDBALoginFlowSubsystem>())
	{
		LoginFlow->StartLoginFlow();
	}
}
