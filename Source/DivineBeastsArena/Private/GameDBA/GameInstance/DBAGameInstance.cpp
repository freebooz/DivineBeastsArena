// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GameInstance/DBAGameInstance.h"

#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/DBAGameUIManager.h"
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

	if (IsFrontendWorld(NewWorld))
	{
		if (UDBAGameUIManager* UIManager = GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->RequestShowLoginFlowWidget();
		}
	}
}

void UDBAGameInstance::StartLoginFlow()
{
	if (UWorld* World = GetWorld())
	{
		if (!IsFrontendWorld(World))
		{
			// In current client startup flow we boot from LobbyMap. If FrontendMap is not available
			// in the running content set, continue login flow in current world instead of failing travel.
			if (!World->GetName().Contains(TEXT("LobbyMap")))
			{
				bPendingStartLoginFlowOnFrontend = true;
				UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] Switching to FrontendMap before login flow."));
				UGameplayStatics::OpenLevel(World, FrontendMapPath);
				return;
			}

			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameInstance] FrontendMap travel skipped; starting login flow in LobbyMap."));
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
