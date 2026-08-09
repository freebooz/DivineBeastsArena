// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 文件职责：前台登录、选角、创建角色流程的 C++ 事件控制器。
- 边界：转发异步用例与状态事件；不创建 Widget、不保存业务规则、不执行网络阻塞调用。
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "DBAFrontendFlowController.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDBAOnFrontendViewStateChanged, EDBALoginFlowState);

/**
 * 旧 UMG 的流程投影适配器。
 * 新页面应直接通过各自 WidgetController 消费 UDBAFrontendFlowSubsystem，禁止创建新的本类实例。
 */
UCLASS(meta = (DeprecatedNode, DeprecationMessage = "请使用页面专属 WidgetController + UDBAFrontendFlowSubsystem。"))
class DIVINEBEASTSARENA_API UDBAFrontendFlowController : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UDBAFrontendFlowSubsystem* InLoginFlow);
	void Deinitialize();

	EDBALoginFlowState GetCurrentState() const;
	const TArray<FDBACharacterSummary>& GetCachedCharacters() const;

	void StartLoginFlow();
	void SubmitLogin(const FString& Email, const FString& Password);
	void SubmitGuestLogin();
	void BeginRegistration();
	void SubmitRegistration(const FString& Account, const FString& Password);
	void CancelRegistration();
	void SetRememberSession(bool bRemember);
	void SubmitCharacterSelection(const FDBACharacterId& CharacterId);
	void SubmitCharacterCreation(const FDBACharacterCreateRequest& Request);
	void EnterCharacterCreate();
	void BackToCharacterSelect();
	void RefreshCharacterList();

	FDBAOnFrontendViewStateChanged OnViewStateChanged;

private:
	UFUNCTION()
	void HandleFrontendStateChanged(EDBAFrontendState PreviousState, EDBAFrontendState NewState);
	static EDBALoginFlowState ToLegacyViewState(EDBAFrontendState State);

	TWeakObjectPtr<UDBAFrontendFlowSubsystem> LoginFlow;
	EDBALoginFlowState CurrentState = EDBALoginFlowState::Booting;
};
