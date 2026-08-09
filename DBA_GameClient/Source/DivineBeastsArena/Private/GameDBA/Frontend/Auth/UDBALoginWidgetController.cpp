// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/Auth/UDBALoginWidgetController.h"

#include "GameDBA/Frontend/Account/DBAOnlineAccountService.h"
#include "GameDBA/Frontend/Auth/DBALoginViewModel.h"
#include "GameDBA/Frontend/Settings/DBAFrontendSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UDBALoginWidgetController::UDBALoginWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBALoginWidgetController::Start()
{
	if (!LoginViewModel)
	{
		LoginViewModel = NewObject<UDBALoginViewModel>(this);
		const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
		LoginViewModel->SetGuestLoginEnabled(!Settings || Settings->bEnableGuestLogin);
		LoginViewModel->SetRememberSession(!Settings || Settings->bRememberSessionByDefault);
	}
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->OnFlowError.RemoveDynamic(this, &UDBALoginWidgetController::HandleFlowError);
		Flow->OnFlowApiError.RemoveDynamic(this, &UDBALoginWidgetController::HandleFlowApiError);
		Flow->OnFlowStateChanged.RemoveDynamic(this, &UDBALoginWidgetController::HandleFlowStateChanged);
		Flow->OnFlowError.AddDynamic(this, &UDBALoginWidgetController::HandleFlowError);
		Flow->OnFlowApiError.AddDynamic(this, &UDBALoginWidgetController::HandleFlowApiError);
		Flow->OnFlowStateChanged.AddDynamic(this, &UDBALoginWidgetController::HandleFlowStateChanged);
		Flow->StartLoginFlow();
	}
}

void UDBALoginWidgetController::LoginWithEmail(const FString& Email, const FString& Password)
{
	if (LoginViewModel && !LoginViewModel->CanSubmit())
	{
		return;
	}
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow())
	{
		if (LoginViewModel)
		{
			LoginViewModel->ClearError();
		}
		Flow->SubmitLogin(Email, Password);
	}
}

void UDBALoginWidgetController::LoginAsGuest()
{
	if (LoginViewModel && (!LoginViewModel->CanSubmit() || !LoginViewModel->IsGuestLoginEnabled()))
	{
		return;
	}
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow())
	{
		if (LoginViewModel)
		{
			LoginViewModel->ClearError();
		}
		Flow->SubmitGuestLogin();
	}
}

void UDBALoginWidgetController::ShowRegistration()
{
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->BeginRegistration();
	}
}

void UDBALoginWidgetController::RegisterWithCredentials(const FString& Account, const FString& Password)
{
	if (LoginViewModel && !LoginViewModel->CanSubmit())
	{
		return;
	}
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow())
	{
		if (LoginViewModel)
		{
			LoginViewModel->ClearError();
		}
		Flow->SubmitRegistration(Account, Password);
	}
}

void UDBALoginWidgetController::CancelRegistration()
{
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->CancelRegistration();
	}
}

void UDBALoginWidgetController::SetRememberSession(const bool bRemember)
{
	if (LoginViewModel)
	{
		LoginViewModel->SetRememberSession(bRemember);
	}
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UDBAOnlineAccountService* AccountService = GameInstance->GetSubsystem<UDBAOnlineAccountService>())
			{
				AccountService->SetRememberSession(bRemember);
			}
		}
	}
}

void UDBALoginWidgetController::HandleFlowError(const FString& ErrorMessage)
{
	OnLoginError.Broadcast(ErrorMessage);
}

void UDBALoginWidgetController::HandleFlowApiError(const FDBAApiError& Error)
{
	if (LoginViewModel)
	{
		LoginViewModel->SetLastError(Error);
	}
	OnLoginApiError.Broadcast(Error);
}

void UDBALoginWidgetController::HandleFlowStateChanged(EDBALoginFlowState State)
{
	if (LoginViewModel)
	{
		LoginViewModel->SetOperationState(State == EDBALoginFlowState::Authenticating
			? EDBAAsyncOperationState::InProgress
			: EDBAAsyncOperationState::Idle);
	}
	OnLoginStateChanged.Broadcast(State);
}

UDBAFrontendFlowSubsystem* UDBALoginWidgetController::GetLoginFlow() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>()
		: nullptr;
}
