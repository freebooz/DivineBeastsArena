// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameMoba/UI/DBAMobaHUDWidgetControllerBase.h"
#include "UDBALoginWidgetController.generated.h"

class UDBALoginViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBALoginUIError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBALoginUIApiError, const FDBAApiError&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBALoginUIStateChanged, EDBALoginFlowState, State);

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBALoginWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBALoginWidgetController(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void Start();

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void LoginWithEmail(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void LoginAsGuest();

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void ShowRegistration();

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void RegisterWithCredentials(const FString& Account, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void CancelRegistration();

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void SetRememberSession(bool bRemember);

	UFUNCTION(BlueprintPure, Category = "DBA|Login")
	UDBALoginViewModel* GetLoginViewModel() const { return LoginViewModel; }

	UPROPERTY(BlueprintAssignable, Category = "DBA|Login")
	FDBALoginUIError OnLoginError;

	/** 新登录 Screen 应消费 ErrorCode/Category/UserMessage，不解析英文错误文本。 */
	UPROPERTY(BlueprintAssignable, Category = "DBA|Login")
	FDBALoginUIApiError OnLoginApiError;

	UPROPERTY(BlueprintAssignable, Category = "DBA|Login")
	FDBALoginUIStateChanged OnLoginStateChanged;

protected:
	UFUNCTION()
	void HandleFlowError(const FString& ErrorMessage);

	UFUNCTION()
	void HandleFlowApiError(const FDBAApiError& Error);

	UFUNCTION()
	void HandleFlowStateChanged(EDBALoginFlowState State);

	UDBAFrontendFlowSubsystem* GetLoginFlow() const;

	UPROPERTY(Transient)
	TObjectPtr<UDBALoginViewModel> LoginViewModel;
};
