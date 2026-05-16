// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "GameCore/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "DBALoginFlowSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDBALoginFlowState : uint8
{
	Startup,
	TryAutoLogin,
	LoginScreen,
	LoadCharacterList,
	CharacterSelect,
	CharacterCreate,
	MainLobby,
	Error
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowStateChanged, EDBALoginFlowState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowCharactersLoaded, const TArray<FDBACharacterSummary>&, Characters);

UCLASS()
class GAMECORE_API UDBALoginFlowSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void StartLoginFlow();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitLogin(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitGuestLogin();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitDebugLogin(const FString& DisplayName = TEXT("frontend_debug"));

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitCharacterSelection(const FDBACharacterId& CharacterId);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitCharacterCreation(const FDBACharacterCreateRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void EnterCharacterCreate();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void BackToCharacterSelect();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void RefreshCharacterList();

	UFUNCTION(BlueprintPure, Category = "DBA|LoginFlow")
	EDBALoginFlowState GetFlowState() const { return FlowState; }

	UFUNCTION(BlueprintPure, Category = "DBA|LoginFlow")
	const TArray<FDBACharacterSummary>& GetCachedCharacters() const { return CachedCharacters; }

	UFUNCTION(BlueprintPure, Category = "DBA|LoginFlow")
	static bool ShouldEnterCharacterCreate(int32 CharacterCount);

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowStateChanged OnFlowStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowError OnFlowError;

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowCharactersLoaded OnCharactersLoaded;

protected:
	UPROPERTY()
	EDBALoginFlowState FlowState = EDBALoginFlowState::Startup;

	UPROPERTY()
	TArray<FDBACharacterSummary> CachedCharacters;

	UPROPERTY()
	EDBAZodiac CurrentSelectedLobbyZodiac = EDBAZodiac::None;

	void SetFlowState(EDBALoginFlowState NewState);
	void LoadCharactersAfterLogin();
	void FetchPostLoginDataAndEnterLobby();
	void EnterMainLobby();
	void BroadcastErrorAndSetState(const FString& ErrorMessage, EDBALoginFlowState NewState);
};
