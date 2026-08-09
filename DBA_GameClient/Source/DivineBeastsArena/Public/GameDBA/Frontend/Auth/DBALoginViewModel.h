// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"
#include "DBALoginViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnLoginViewModelChanged);

/**
 * 登录与注册页面的可绑定显示状态。
 * 不保存账号、密码或任何 Token；输入控件只将业务意图提交给 Controller。
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBALoginViewModel : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DBA|Login")
	EDBAAsyncOperationState GetOperationState() const { return OperationState; }

	UFUNCTION(BlueprintPure, Category = "DBA|Login")
	const FDBAApiError& GetLastError() const { return LastError; }

	UFUNCTION(BlueprintPure, Category = "DBA|Login")
	bool CanSubmit() const { return OperationState != EDBAAsyncOperationState::InProgress; }

	UFUNCTION(BlueprintPure, Category = "DBA|Login")
	bool IsGuestLoginEnabled() const { return bGuestLoginEnabled; }

	UFUNCTION(BlueprintPure, Category = "DBA|Login")
	bool ShouldRememberSession() const { return bRememberSession; }

	void SetOperationState(EDBAAsyncOperationState InOperationState);
	void SetLastError(const FDBAApiError& InError);
	void ClearError();
	void SetGuestLoginEnabled(bool bEnabled);
	void SetRememberSession(bool bRemember);

	UPROPERTY(BlueprintAssignable, Category = "DBA|Login")
	FDBAOnLoginViewModelChanged OnChanged;

private:
	UPROPERTY(VisibleAnywhere, Category = "DBA|Login")
	EDBAAsyncOperationState OperationState = EDBAAsyncOperationState::Idle;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Login")
	FDBAApiError LastError;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Login")
	bool bGuestLoginEnabled = true;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Login")
	bool bRememberSession = true;
};
