// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"
#include "GameDBA/Core/DBALogChannels.h"

UDBALoginFlowWidgetBase::UDBALoginFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBALoginFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBALoginFlowWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	ClearError();
}

void UDBALoginFlowWidgetBase::ShowError(const FString& ErrorMessage)
{
	LastErrorMessage = ErrorMessage;
	BP_OnShowError(ErrorMessage);
	UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] 显示错误: %s"), *ErrorMessage);
}

void UDBALoginFlowWidgetBase::ClearError()
{
	LastErrorMessage.Empty();
	BP_OnClearError();
}
