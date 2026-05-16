// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GameInstance/DBAGameInstance.h"

#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/DBAGameUIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"

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

	bool IsLobbyWorld(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("LobbyMap"))
			|| LevelPath.Contains(TEXT("MainLobby"));
	}

	FName GetConfiguredMapPath(const TCHAR* ConfigKey, const FName& FallbackPath)
	{
		FString ConfigValue;
		if (GConfig && GConfig->GetString(TEXT("/Script/DivineBeastsArena.DBAFrontendConfig"), ConfigKey, ConfigValue, GGameIni))
		{
			ConfigValue = ConfigValue.TrimStartAndEnd();
			if (!ConfigValue.IsEmpty())
			{
				return FName(*ConfigValue);
			}
		}
		return FallbackPath;
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

	if (IsDedicatedServerInstance())
	{
		return;
	}

	if (bPendingStartLoginFlowOnFrontend && IsFrontendWorld(NewWorld))
	{
		bPendingStartLoginFlowOnFrontend = false;
		StartLoginFlow();
	}

	UDBALoginFlowSubsystem* LoginFlow = GetSubsystem<UDBALoginFlowSubsystem>();
	UDBAGameUIManager* UIManager = GetSubsystem<UDBAGameUIManager>();
	const EDBALoginFlowState FlowState = LoginFlow ? LoginFlow->GetFlowState() : EDBALoginFlowState::Startup;

	if (IsFrontendWorld(NewWorld) && FlowState != EDBALoginFlowState::MainLobby)
	{
		if (UIManager)
		{
			UIManager->RequestShowLoginFlowWidget();
		}
		return;
	}

	if (FlowState == EDBALoginFlowState::MainLobby && (IsLobbyWorld(NewWorld) || IsFrontendWorld(NewWorld)))
	{
		if (UIManager)
		{
			UIManager->RequestShowLoginFlowWidget();
		}
	}
}

void UDBAGameInstance::StartLoginFlow()
{
	if (IsDedicatedServerInstance())
	{
		UE_LOG(LogDBACore, Verbose, TEXT("[DBAGameInstance] Dedicated server instance skips frontend login flow."));
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (!IsFrontendWorld(World))
		{
			bPendingStartLoginFlowOnFrontend = true;
			const FName ConfiguredFrontendMapPath = GetConfiguredMapPath(TEXT("DefaultFrontendMap"), FrontendMapPath);
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] Switching to frontend map before login flow: %s"), *ConfiguredFrontendMapPath.ToString());
			UGameplayStatics::OpenLevel(World, ConfiguredFrontendMapPath);
			return;
		}
	}

	if (bLoginFlowStarted)
	{
		if (UDBALoginFlowSubsystem* LoginFlow = GetSubsystem<UDBALoginFlowSubsystem>())
		{
			if (LoginFlow->GetFlowState() == EDBALoginFlowState::Startup)
			{
				LoginFlow->StartLoginFlow();
			}
		}
		if (UDBAGameUIManager* UIManager = GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->RequestShowLoginFlowWidget();
		}
		return;
	}

	bLoginFlowStarted = true;
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] Start login flow."));

	if (UDBALoginFlowSubsystem* LoginFlow = GetSubsystem<UDBALoginFlowSubsystem>())
	{
		LoginFlow->StartLoginFlow();
	}

	if (UDBAGameUIManager* UIManager = GetSubsystem<UDBAGameUIManager>())
	{
		UIManager->RequestShowLoginFlowWidget();
	}
}
