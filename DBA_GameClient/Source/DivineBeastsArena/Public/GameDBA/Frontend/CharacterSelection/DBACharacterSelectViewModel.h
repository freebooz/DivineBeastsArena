// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Networking/Account/DBAAccountTypes.h"
#include "GameDBA/Frontend/Character/DBACharacterRosterSubsystem.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"
#include "DBACharacterSelectViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnCharacterSelectViewModelChanged);

/** 角色选择 Screen 的可绑定显示状态；不保存 HTTP DTO、Token 或 UObject 角色实例。 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterSelectViewModel : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	const TArray<FDBACharacterSummary>& GetCharacters() const { return Characters; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	const FDBACharacterId& GetSelectedCharacterId() const { return SelectedCharacterId; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	const FDBACharacterDetails& GetSelectedDetails() const { return SelectedDetails; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	bool HasCharacters() const { return !Characters.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	bool IsRosterLoading() const { return bRosterLoading; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	bool IsPreviewLoading() const { return bPreviewLoading; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	bool IsDeleteConfirmationVisible() const { return bDeleteConfirmationVisible; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	bool CanEnterGame() const { return SelectedCharacterId.IsValid() && !bRosterLoading && !bPreviewLoading; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	const FDBAApiError& GetLastError() const { return LastError; }

	void ApplyRoster(const TArray<FDBACharacterSummary>& InCharacters, const FDBACharacterDetails* PreferredDetails);
	void SelectCharacter(const FDBACharacterDetails& Details);
	void SetRosterLoading(bool bLoading);
	void SetPreviewLoading(bool bLoading);
	void SetDeleteConfirmationVisible(bool bVisible);
	void SetLastError(const FDBAApiError& Error);
	void ClearError();

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterSelect")
	FDBAOnCharacterSelectViewModelChanged OnChanged;

private:
	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterSelect")
	TArray<FDBACharacterSummary> Characters;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterSelect")
	FDBACharacterId SelectedCharacterId;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterSelect")
	FDBACharacterDetails SelectedDetails;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterSelect")
	FDBAApiError LastError;

	bool bRosterLoading = false;
	bool bPreviewLoading = false;
	bool bDeleteConfirmationVisible = false;
};
