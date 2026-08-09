// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/Frontend/Startup/DBAStartupViewModel.h"
#include "DBAStartupCoordinatorSubsystem.generated.h"

class UDBAStartupVideoWidget;

UENUM(BlueprintType)
enum class EDBAStartupPhase : uint8
{
	NotStarted,
	ReadingConfiguration,
	LoadingLocalPreferences,
	PreparingSecureSession,
	CheckingBackend,
	TravellingToFrontend,
	AwaitingContinue,
	Completed,
	RecoverableFailure,
	FatalFailure
};

/** Owns process-start to persistent-frontend handoff and never loads character presentation resources. */
UCLASS()
class DIVINEBEASTSARENA_API UDBAStartupCoordinatorSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void OnSubsystemDeinitialize() override;

	void BeginStartup();
	void HandleWorldChanged(UWorld* NewWorld);

	UFUNCTION(BlueprintPure, Category = "DBA|Startup")
	EDBAStartupPhase GetStartupPhase() const { return StartupPhase; }

	UFUNCTION(BlueprintPure, Category = "DBA|Startup")
	UDBAStartupViewModel* GetViewModel() const { return ViewModel; }

	bool ShouldHoldFrontendFlow() const
	{
		return StartupPhase != EDBAStartupPhase::NotStarted && StartupPhase != EDBAStartupPhase::Completed;
	}

	UFUNCTION(BlueprintCallable, Category = "DBA|Startup")
	void RetryBackendCheck();

private:
	bool IsClientRuntime() const;
	bool ValidateConfiguration(FName& OutFrontendMapPath) const;
	void LoadLocalPreferences();
	void PrepareSecureSession();
	void StartBackendCheck();
	void CompleteBackendCheck(bool bOnline, const FText& StatusText);
	void BeginFrontendTravel();
	void PresentStartupScreen();
	void FinishStartupAndEnterFlow();
	void EnterFatalFailure(const FText& FailureText);
	void SetPhase(EDBAStartupPhase NewPhase);

	UFUNCTION()
	void HandleBackendVersionCheck(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleBackendCheckTimeout();

	UFUNCTION()
	void HandleStartupContinueRequested();

	UPROPERTY(Transient)
	TObjectPtr<UDBAStartupViewModel> ViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UDBAStartupVideoWidget> StartupScreen;

	UPROPERTY(Transient)
	EDBAStartupPhase StartupPhase = EDBAStartupPhase::NotStarted;

	FName FrontendMapPath;
	FTimerHandle BackendCheckTimeoutHandle;
	bool bBackendCheckResolved = false;
	bool bFrontendTravelRequested = false;
};
