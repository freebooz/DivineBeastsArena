// Copyright FreeboozStudio. All Rights Reserved.

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
	bool ShouldFallback(EDBAOnlineAccountError Error) const;

	void FallbackLogin(FDBAOnLoginComplete OnComplete);
	void FallbackAutoLogin(FDBAOnLoginComplete OnComplete);
	void FallbackCharacterList(FDBAOnCharacterListLoaded OnComplete);
	void FallbackCreateCharacter(const FDBACharacterCreateRequest& Request, FDBAOnCharacterCreated OnComplete);
};
