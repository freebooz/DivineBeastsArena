// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 文件职责：登录界面的用户动作、错误状态与登录流程事件响应。
- 边界：只调用 UDBAFrontendFlowSubsystem 的异步用例并响应事件；不负责布局构建和资源加载。
*/

#include "GameDBA/Frontend/Auth/UDBALoginFlowWidgetBase.h"

#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/Controllers/DBAGameUIManager.h"
#include "GameDBA/UI/Frontend/DBAFrontendFlowController.h"

namespace
{
	FText GetLoginFlowStatusText(const EDBALoginFlowState State)
	{
		switch (State)
		{
		case EDBALoginFlowState::Authenticating:
			return NSLOCTEXT("DBALoginFlowWidget", "SigningIn", "正在登录...");
		case EDBALoginFlowState::LoadingCharacters:
			return NSLOCTEXT("DBALoginFlowWidget", "LoadingCharacters", "正在加载角色...");
		case EDBALoginFlowState::CharacterSelecting:
			return NSLOCTEXT("DBALoginFlowWidget", "ChooseCharacter", "请选择角色。");
		case EDBALoginFlowState::CharacterCreating:
			return NSLOCTEXT("DBALoginFlowWidget", "CreateCharacter", "请创建角色。");
		case EDBALoginFlowState::AllocatingVillage:
		case EDBALoginFlowState::WaitingVillageServer:
		case EDBALoginFlowState::ConnectingVillage:
		case EDBALoginFlowState::InitializingVillage:
			return NSLOCTEXT("DBALoginFlowWidget", "EnteringVillage", "正在进入新手村...");
		case EDBALoginFlowState::InVillage:
			return NSLOCTEXT("DBALoginFlowWidget", "VillageReady", "新手村已就绪。");
		default:
			return FText::GetEmpty();
		}
	}
}
void UDBALoginFlowWidgetBase::SubmitLogin()
{
	SetStatus(NSLOCTEXT("DBALoginFlowWidget", "LoginRequestReceived", "已收到登录请求，正在校验输入..."));
	if (!CanSubmitLoginAction())
	{
		return;
	}

	const FString Email = EmailInput ? EmailInput->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Password = PasswordInput ? PasswordInput->GetText().ToString() : FString();

	if (Email.IsEmpty() || Password.IsEmpty())
	{
		ShowError(TEXT("请输入账号和密码。"));
		return;
	}

	if (UDBAFrontendFlowController* FlowController = GetFrontendFlowController())
	{
		ClearError();
		SetStatus(NSLOCTEXT("DBALoginFlowWidget", "SigningIn", "登录验证中..."));
		FlowController->SubmitLogin(Email, Password);
	}
	else
	{
		ShowError(TEXT("登录流程不可用。"));
	}
}

void UDBALoginFlowWidgetBase::SubmitGuestLogin()
{
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 点击游客登录"));
	if (!CanSubmitLoginAction())
	{
		return;
	}

	if (UDBAFrontendFlowController* FlowController = GetFrontendFlowController())
	{
		ClearError();
		SetStatus(NSLOCTEXT("DBALoginFlowWidget", "SigningInGuest", "\u8bbf\u5ba2\u767b\u5f55\u4e2d..."));
		FlowController->SubmitGuestLogin();
	}
	else
	{
		ShowError(TEXT("\u767b\u5f55\u6d41\u7a0b\u4e0d\u53ef\u7528\u3002"));
	}
}

void UDBALoginFlowWidgetBase::ShowError(const FString& ErrorMessage)
{
	LastErrorMessage = ErrorMessage;
	if (ErrorText)
	{
		ErrorText->SetText(FText::FromString(ErrorMessage));
		ErrorText->SetVisibility(ErrorMessage.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	BP_OnShowError(ErrorMessage);
	UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] 错误: %s"), *ErrorMessage);
}

void UDBALoginFlowWidgetBase::ClearError()
{
	LastErrorMessage.Empty();
	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
		ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	}
	BP_OnClearError();
}

void UDBALoginFlowWidgetBase::HandleLoginClicked()
{
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 已点击登录按钮。"));
	SubmitLogin();
}

void UDBALoginFlowWidgetBase::HandleGuestLoginClicked()
{
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 点击游客登录按钮"));
	SubmitGuestLogin();
}

void UDBALoginFlowWidgetBase::HandleRememberToggleClicked()
{
	bRememberAccount = !bRememberAccount;
	if (UDBAFrontendFlowController* FlowController = GetFrontendFlowController())
	{
		FlowController->SetRememberSession(bRememberAccount);
	}
	UpdateReferenceToggleVisuals();
	SetStatus(bRememberAccount ? FText::FromString(TEXT("已开启记住账号。")) : FText::FromString(TEXT("已关闭记住账号。")));
}

void UDBALoginFlowWidgetBase::HandleAgreementToggleClicked()
{
	bAgreementAccepted = !bAgreementAccepted;
	UpdateReferenceToggleVisuals();
	if (bAgreementAccepted)
	{
		ClearError();
		SetStatus(FText::FromString(TEXT("已同意用户协议与隐私政策。")));
	}
	else
	{
		SetStatus(FText::FromString(TEXT("请阅读并同意协议后继续。")));
	}
}

void UDBALoginFlowWidgetBase::HandlePasswordVisibilityClicked()
{
	bPasswordVisible = !bPasswordVisible;
	if (PasswordInput)
	{
		PasswordInput->SetIsPassword(!bPasswordVisible);
	}
	UpdateReferenceToggleVisuals();
}

void UDBALoginFlowWidgetBase::HandleServerSelectClicked()
{
	if (AvailableServers.IsEmpty())
	{
		return;
	}

	SelectedServerIndex = (SelectedServerIndex + 1) % AvailableServers.Num();
	UpdateReferenceToggleVisuals();
	SetStatus(FText::Format(FText::FromString(TEXT("已选择服务器：{0}")), AvailableServers[SelectedServerIndex]));
}

void UDBALoginFlowWidgetBase::HandleForgotPasswordClicked()
{
	SetStatus(FText::FromString(TEXT("密码找回功能暂未开放。")));
}

void UDBALoginFlowWidgetBase::HandleRegisterAccountClicked()
{
	if (UDBAFrontendFlowController* FlowController = GetFrontendFlowController())
	{
		ClearError();
		FlowController->BeginRegistration();
	}
	else
	{
		ShowError(TEXT("注册流程不可用。"));
	}
}

void UDBALoginFlowWidgetBase::HandleAnnouncementClicked()
{
	SetStatus(FText::FromString(TEXT("公告中心暂未开放。")));
}

void UDBALoginFlowWidgetBase::HandleSupportClicked()
{
	SetStatus(FText::FromString(TEXT("客服系统暂未开放，请稍后再试。")));
}

void UDBALoginFlowWidgetBase::HandleRepairClicked()
{
	if (EmailInput)
	{
		EmailInput->SetText(FText::GetEmpty());
	}
	if (PasswordInput)
	{
		PasswordInput->SetText(FText::GetEmpty());
		PasswordInput->SetIsPassword(!bPasswordVisible);
	}
	ClearError();
	SetStatus(FText::FromString(TEXT("已完成登录界面修复检查。")));
}

void UDBALoginFlowWidgetBase::HandleUserAgreementClicked()
{
	SetStatus(FText::FromString(TEXT("用户协议内容暂未配置。")));
}

void UDBALoginFlowWidgetBase::HandlePrivacyPolicyClicked()
{
	SetStatus(FText::FromString(TEXT("隐私政策内容暂未配置。")));
}

void UDBALoginFlowWidgetBase::HandleFlowStateChanged(EDBALoginFlowState NewState)
{
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 流程状态变更：%d"), static_cast<int32>(NewState));
	SetStatus(GetLoginFlowStatusText(NewState));
	UpdateLoadingStateByFlow(NewState);
	if (NewState != EDBALoginFlowState::AwaitingLogin
		&& NewState != EDBALoginFlowState::RecoverableError
		&& NewState != EDBALoginFlowState::FatalError)
	{
		ClearError();
	}
	BP_OnFlowStateChanged(NewState);
}

void UDBALoginFlowWidgetBase::HandleFlowError(const FString& ErrorMessage)
{
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->HideLobbyLoadingScreen();
		}
	}
	ShowError(ErrorMessage);
}
