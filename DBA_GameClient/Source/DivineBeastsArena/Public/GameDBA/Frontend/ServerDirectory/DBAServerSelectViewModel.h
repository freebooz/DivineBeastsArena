// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"
#include "GameDBA/Frontend/ServerDirectory/DBAServerDirectorySubsystem.h"
#include "DBAServerSelectViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnServerSelectViewModelChanged);

/** UI 专用展示模型，不暴露 HTTP DTO 或原始 JSON。 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAServerSelectItemViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerSelect")
	FString ServerId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerSelect")
	FText Name;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerSelect")
	FText RegionText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerSelect")
	FText StatusText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerSelect")
	FText PopulationText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerSelect")
	FText UnavailableReason;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerSelect")
	bool bRecommended = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerSelect")
	bool bIsLastLoginServer = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerSelect")
	bool bCanSelect = false;
};

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAServerSelectViewModel : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DBA|ServerSelect")
	const TArray<FDBAServerSelectItemViewData>& GetServers() const { return Servers; }

	UFUNCTION(BlueprintPure, Category = "DBA|ServerSelect")
	const FDBAApiError& GetLastError() const { return LastError; }

	UFUNCTION(BlueprintPure, Category = "DBA|ServerSelect")
	EDBAAsyncOperationState GetOperationState() const { return OperationState; }

	UFUNCTION(BlueprintPure, Category = "DBA|ServerSelect")
	FString GetSelectedServerId() const { return SelectedServerId; }

	UFUNCTION(BlueprintPure, Category = "DBA|ServerSelect")
	bool CanRefresh() const { return OperationState != EDBAAsyncOperationState::InProgress; }

	UFUNCTION(BlueprintPure, Category = "DBA|ServerSelect")
	bool CanConfirmSelection() const;

	UFUNCTION(BlueprintPure, Category = "DBA|ServerSelect")
	bool IsEmpty() const { return Servers.IsEmpty(); }

	void ApplyDirectory(const TArray<FDBAServerDirectoryEntry>& InServers, const FString& LastServerId);
	void SetOperationState(EDBAAsyncOperationState InOperationState);
	void SetLastError(const FDBAApiError& InError);
	bool SelectServer(const FString& ServerId);

	UPROPERTY(BlueprintAssignable, Category = "DBA|ServerSelect")
	FDBAOnServerSelectViewModelChanged OnChanged;

private:
	static FDBAServerSelectItemViewData MakeViewData(const FDBAServerDirectoryEntry& Entry, const FString& LastServerId);

	UPROPERTY(VisibleAnywhere, Category = "DBA|ServerSelect")
	TArray<FDBAServerSelectItemViewData> Servers;

	UPROPERTY(VisibleAnywhere, Category = "DBA|ServerSelect")
	EDBAAsyncOperationState OperationState = EDBAAsyncOperationState::Idle;

	UPROPERTY(VisibleAnywhere, Category = "DBA|ServerSelect")
	FDBAApiError LastError;

	UPROPERTY(VisibleAnywhere, Category = "DBA|ServerSelect")
	FString SelectedServerId;
};
