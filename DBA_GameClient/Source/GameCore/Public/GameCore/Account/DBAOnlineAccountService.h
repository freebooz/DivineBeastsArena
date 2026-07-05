// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountServiceBase.h"
#include "GameCore/Account/DBAOnlineAccountTypes.h"
#include "DBAOnlineAccountService.generated.h"

class IHttpRequest;
class IHttpResponse;
class UDBAMockAccountService;

UCLASS()
class GAMECORE_API UDBAOnlineAccountService : public UDBAAccountServiceBase
{
	GENERATED_BODY()

public:
	UDBAOnlineAccountService();

	virtual void OnSubsystemInitialize() override;
	virtual void CancelAllAsyncOperations() override;

	virtual void Login(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete) override;
	virtual void Register(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete) override;
	virtual void GuestLogin(FDBAOnLoginComplete OnComplete) override;
	virtual void AutoLogin(FDBAOnLoginComplete OnComplete) override;
	virtual void Logout(FDBAOnLogoutComplete OnComplete) override;
	virtual void GetCharacterList(FDBAOnCharacterListLoaded OnComplete) override;
	virtual void CreateCharacter(const FDBACharacterCreateRequest& Request, FDBAOnCharacterCreated OnComplete) override;
	virtual void SelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete) override;

	UFUNCTION(BlueprintPure, Category = "Account|Online")
	static bool CanFallbackToMock(EDBAOnlineAccountError Error);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account|Online")
	FDBAOnlineAccountConfig OnlineConfig;

	UPROPERTY()
	TObjectPtr<UDBAMockAccountService> MockService;

	TArray<TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> PendingRequests;

	FString BuildUrl(const FString& Path) const;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateJsonRequest(const FString& Verb, const FString& Path, const FString& Body);
	EDBAOnlineAccountError ClassifyHttpFailure(bool bSucceeded, const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response) const;

	void CacheLoginSuccess(const FDBALoginResponse& Response);
	void LoadOnlineAccountState();
	void SaveOnlineAccountState(const TArray<FDBACharacterSummary>* Characters = nullptr);
	bool ShouldFallback(EDBAOnlineAccountError Error) const;

	void FallbackLogin(FDBAOnLoginComplete OnComplete);
	void FallbackAutoLogin(FDBAOnLoginComplete OnComplete);
	void FallbackCharacterList(FDBAOnCharacterListLoaded OnComplete);
	void FallbackCreateCharacter(const FDBACharacterCreateRequest& Request, FDBAOnCharacterCreated OnComplete);
	void FallbackSelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete);
};
