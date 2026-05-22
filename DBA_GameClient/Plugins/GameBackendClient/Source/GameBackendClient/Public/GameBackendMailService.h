// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DBA_GameBackendTypes.h"
#include "DBA_GameBackendMailService.generated.h"

class UDBA_GameBackendClientSubsystem;
class FDBA_GameBackendHttpClient;

UCLASS(BlueprintType)
class GAMEBACKENDCLIENT_API UDBA_GameBackendMailService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Mail")
	void GetMyMails(const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Mail")
	void GetMailDetail(const FString& MailId, const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Mail")
	void MarkRead(const FString& MailId, const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Mail")
	void ClaimMail(const FString& MailId, const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Mail")
	void ClaimAll(const FDBA_GameBackendResponseDelegate& Callback);

private:
	TWeakObjectPtr<UDBA_GameBackendClientSubsystem> Subsystem;
	FDBA_GameBackendHttpClient* HttpClient = nullptr;
};
