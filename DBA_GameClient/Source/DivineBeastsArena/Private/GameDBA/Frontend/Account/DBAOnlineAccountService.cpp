// Copyright FreeboozStudio. All Rights Reserved.

#include "GameDBA/Frontend/Account/DBAOnlineAccountService.h"

#include "GameBackendAuthService.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendPlayerService.h"
#include "GameCore/Data/Profile/DBAAccountSaveGame.h"
#include "GameDBA/Frontend/Account/DBAOnlineAccountJson.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformProcess.h"

namespace
{
FString BuildStableGuestDeviceId()
{
	const FString ComputerName = FPlatformProcess::ComputerName();
	return ComputerName.IsEmpty()
		? TEXT("DBA_LOCAL_DEVICE")
		: FString::Printf(TEXT("DBA_%s"), *ComputerName);
}

FDBALoginResponse MakeLoginFailure(const FString& Context, const FString& ErrorMessage)
{
	FDBALoginResponse Response;
	Response.ErrorMessage = ErrorMessage.IsEmpty()
		? Context
		: FString::Printf(TEXT("%s：%s"), *Context, *ErrorMessage);
	return Response;
}
}

UDBAOnlineAccountService::UDBAOnlineAccountService()
{
}

void UDBAOnlineAccountService::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

	LoadOnlineAccountState();
	if (UDBA_GameBackendClientSubsystem* BackendClient = GetBackendClient())
	{
		BackendClient->SetAuthTokens(SessionToken, RefreshToken, CurrentAccountInfo.AccountId.ToString());
	}

	LogSubsystemInfo(TEXT("在线账号服务已初始化，传输由 GameBackendClient 统一管理。"));
}

void UDBAOnlineAccountService::CancelAllAsyncOperations()
{
	++RequestGeneration;
}

UDBA_GameBackendClientSubsystem* UDBAOnlineAccountService::GetBackendClient() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr;
}

bool UDBAOnlineAccountService::IsRequestCurrent(uint64 Generation) const
{
	return Generation == RequestGeneration && IsValid(this);
}

void UDBAOnlineAccountService::CacheLoginSuccess(const FDBALoginResponse& Response)
{
	CurrentAccountInfo = Response.AccountInfo;
	SessionToken = Response.SessionToken;
	RefreshToken = Response.RefreshToken;
	SaveOnlineAccountState();
}

void UDBAOnlineAccountService::LoadOnlineAccountState()
{
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame || !SaveGame->IsValid())
	{
		return;
	}

	CurrentAccountInfo = SaveGame->AccountInfo;
	CurrentCharacterId = SaveGame->CurrentCharacterId;
	SessionToken = SaveGame->SessionToken;
	RefreshToken = SaveGame->RefreshToken;
}

void UDBAOnlineAccountService::SaveOnlineAccountState(const TArray<FDBACharacterSummary>* Characters)
{
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame)
	{
		SaveGame = CreateDefaultAccountSaveGame();
	}
	if (!SaveGame)
	{
		return;
	}

	SaveGame->AccountInfo = CurrentAccountInfo;
	SaveGame->CurrentCharacterId = CurrentCharacterId;
	SaveGame->SessionToken = SessionToken;
	SaveGame->RefreshToken = RefreshToken;
	if (Characters)
	{
		SaveGame->Characters = *Characters;
	}
	SaveAccountSaveGame(SaveGame);
}

void UDBAOnlineAccountService::Login(const FDBALoginRequest& RequestData, FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Login")))
	{
		return;
	}
	if (RequestData.LoginType == EDBALoginType::Guest)
	{
		GuestLogin(OnComplete);
		return;
	}

	UDBA_GameBackendClientSubsystem* BackendClient = GetBackendClient();
	UDBA_GameBackendAuthService* AuthService = BackendClient ? BackendClient->GetAuthService() : nullptr;
	if (!AuthService)
	{
		OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("登录失败"), TEXT("后端鉴权服务不可用")));
		return;
	}

	const uint64 Generation = RequestGeneration;
	AuthService->AccountLoginAsync(RequestData.Email, RequestData.Password,
		[WeakThis = TWeakObjectPtr<UDBAOnlineAccountService>(this), Generation, OnComplete](bool bSuccess, const FString& ErrorMessage, const FDBA_GameBackendAuthTokens& Tokens, const FString& RawResponse)
		{
			UDBAOnlineAccountService* Service = WeakThis.Get();
			if (!Service || !Service->IsRequestCurrent(Generation))
			{
				return;
			}

			if (!bSuccess)
			{
				OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("登录失败"), ErrorMessage));
				return;
			}

			FDBALoginResponse Response;
			FString ParseError;
			if (!FDBAOnlineAccountJson::ParseLoginResponse(RawResponse, Response, ParseError))
			{
				OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("登录响应解析失败"), ParseError));
				return;
			}

			Response.bSuccess = true;
			Response.SessionToken = Tokens.AccessToken;
			Response.RefreshToken = Tokens.RefreshToken;
			Response.PlayerId = Tokens.PlayerId;
			Response.AccountInfo.AccountId = FDBAAccountId(Tokens.PlayerId);
			Response.AccountInfo.LoginType = EDBALoginType::Email;
			Response.AccountInfo.Status = EDBAAccountStatus::Normal;
			Service->CacheLoginSuccess(Response);
			OnComplete.ExecuteIfBound(Response);
		});
}

void UDBAOnlineAccountService::Register(const FDBALoginRequest& RequestData, FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Register")))
	{
		return;
	}

	UDBA_GameBackendClientSubsystem* BackendClient = GetBackendClient();
	UDBA_GameBackendAuthService* AuthService = BackendClient ? BackendClient->GetAuthService() : nullptr;
	if (!AuthService)
	{
		OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("注册失败"), TEXT("后端鉴权服务不可用")));
		return;
	}

	const uint64 Generation = RequestGeneration;
	AuthService->AccountRegisterAsync(RequestData.Email, RequestData.Password, RequestData.Email,
		[WeakThis = TWeakObjectPtr<UDBAOnlineAccountService>(this), Generation, OnComplete](bool bSuccess, const FString& ErrorMessage, const FDBA_GameBackendAuthTokens& Tokens, const FString& RawResponse)
		{
			UDBAOnlineAccountService* Service = WeakThis.Get();
			if (!Service || !Service->IsRequestCurrent(Generation))
			{
				return;
			}
			if (!bSuccess)
			{
				OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("注册失败"), ErrorMessage));
				return;
			}

			FDBALoginResponse Response;
			FString ParseError;
			if (!FDBAOnlineAccountJson::ParseLoginResponse(RawResponse, Response, ParseError))
			{
				OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("注册响应解析失败"), ParseError));
				return;
			}

			Response.bSuccess = true;
			Response.SessionToken = Tokens.AccessToken;
			Response.RefreshToken = Tokens.RefreshToken;
			Response.PlayerId = Tokens.PlayerId;
			Response.AccountInfo.AccountId = FDBAAccountId(Tokens.PlayerId);
			Response.AccountInfo.LoginType = EDBALoginType::Email;
			Response.AccountInfo.Status = EDBAAccountStatus::Normal;
			Service->CacheLoginSuccess(Response);
			OnComplete.ExecuteIfBound(Response);
		});
}

void UDBAOnlineAccountService::GuestLogin(FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("GuestLogin")))
	{
		return;
	}

	UDBA_GameBackendClientSubsystem* BackendClient = GetBackendClient();
	UDBA_GameBackendAuthService* AuthService = BackendClient ? BackendClient->GetAuthService() : nullptr;
	if (!AuthService)
	{
		OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("游客登录失败"), TEXT("后端鉴权服务不可用")));
		return;
	}

	FDBA_GameBackendGuestLoginRequest Request;
	Request.DeviceId = BuildStableGuestDeviceId();
	Request.DeviceName = FPlatformProcess::ComputerName();
	Request.Platform = BackendClient->GetPlatformName();

	const uint64 Generation = RequestGeneration;
	AuthService->GuestLoginAsync(Request,
		[WeakThis = TWeakObjectPtr<UDBAOnlineAccountService>(this), Generation, OnComplete](bool bSuccess, const FString& ErrorMessage, const FDBA_GameBackendAuthTokens& Tokens, const FString& RawResponse)
		{
			UDBAOnlineAccountService* Service = WeakThis.Get();
			if (!Service || !Service->IsRequestCurrent(Generation))
			{
				return;
			}
			if (!bSuccess)
			{
				OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("游客登录失败"), ErrorMessage));
				return;
			}

			FDBALoginResponse Response;
			FString ParseError;
			if (!FDBAOnlineAccountJson::ParseLoginResponse(RawResponse, Response, ParseError))
			{
				OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("游客登录响应解析失败"), ParseError));
				return;
			}

			Response.bSuccess = true;
			Response.SessionToken = Tokens.AccessToken;
			Response.RefreshToken = Tokens.RefreshToken;
			Response.PlayerId = Tokens.PlayerId;
			Response.AccountInfo.AccountId = FDBAAccountId(Tokens.PlayerId);
			Response.AccountInfo.LoginType = EDBALoginType::Guest;
			Response.AccountInfo.Status = EDBAAccountStatus::Normal;
			Service->CacheLoginSuccess(Response);
			OnComplete.ExecuteIfBound(Response);
		});
}

void UDBAOnlineAccountService::AutoLogin(FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("AutoLogin")))
	{
		return;
	}
	if (RefreshToken.IsEmpty())
	{
		LoadOnlineAccountState();
	}
	if (RefreshToken.IsEmpty())
	{
		OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("自动登录失败"), TEXT("没有可刷新的登录令牌")));
		return;
	}

	UDBA_GameBackendClientSubsystem* BackendClient = GetBackendClient();
	UDBA_GameBackendAuthService* AuthService = BackendClient ? BackendClient->GetAuthService() : nullptr;
	if (!AuthService)
	{
		OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("自动登录失败"), TEXT("后端鉴权服务不可用")));
		return;
	}
	BackendClient->SetAuthTokens(SessionToken, RefreshToken, CurrentAccountInfo.AccountId.ToString());

	const uint64 Generation = RequestGeneration;
	AuthService->RefreshTokenAsync(
		[WeakThis = TWeakObjectPtr<UDBAOnlineAccountService>(this), Generation, OnComplete](bool bSuccess, const FString& ErrorMessage, const FDBA_GameBackendAuthTokens& Tokens, const FString& RawResponse)
		{
			UDBAOnlineAccountService* Service = WeakThis.Get();
			if (!Service || !Service->IsRequestCurrent(Generation))
			{
				return;
			}
			if (!bSuccess)
			{
				OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("自动登录失败"), ErrorMessage));
				return;
			}

			FDBALoginResponse Response;
			FString ParseError;
			if (!FDBAOnlineAccountJson::ParseLoginResponse(RawResponse, Response, ParseError))
			{
				OnComplete.ExecuteIfBound(MakeLoginFailure(TEXT("自动登录响应解析失败"), ParseError));
				return;
			}

			Response.bSuccess = true;
			Response.SessionToken = Tokens.AccessToken;
			Response.RefreshToken = Tokens.RefreshToken;
			Response.PlayerId = Tokens.PlayerId;
			Response.AccountInfo = Service->CurrentAccountInfo;
			Service->CacheLoginSuccess(Response);
			OnComplete.ExecuteIfBound(Response);
		});
}

void UDBAOnlineAccountService::Logout(FDBAOnLogoutComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Logout")))
	{
		return;
	}

	const auto FinishLogout = [WeakThis = TWeakObjectPtr<UDBAOnlineAccountService>(this), OnComplete]()
	{
		UDBAOnlineAccountService* Service = WeakThis.Get();
		if (!Service)
		{
			return;
		}

		Service->CurrentAccountInfo = FDBAAccountInfo();
		Service->CurrentCharacterId = FDBACharacterId();
		Service->SessionToken.Empty();
		Service->RefreshToken.Empty();
		if (UDBA_GameBackendClientSubsystem* BackendClient = Service->GetBackendClient())
		{
			BackendClient->ClearAuthTokens();
		}
		Service->SaveOnlineAccountState();
		OnComplete.ExecuteIfBound();
	};

	UDBA_GameBackendClientSubsystem* BackendClient = GetBackendClient();
	UDBA_GameBackendAuthService* AuthService = BackendClient ? BackendClient->GetAuthService() : nullptr;
	if (!AuthService || SessionToken.IsEmpty())
	{
		FinishLogout();
		return;
	}

	const uint64 Generation = RequestGeneration;
	AuthService->LogoutAsync([WeakThis = TWeakObjectPtr<UDBAOnlineAccountService>(this), Generation, FinishLogout](bool, const FString&, const FString&)
	{
		UDBAOnlineAccountService* Service = WeakThis.Get();
		if (Service && Service->IsRequestCurrent(Generation))
		{
			FinishLogout();
		}
	});
}

void UDBAOnlineAccountService::GetCharacterList(FDBAOnCharacterListLoaded OnComplete)
{
	UDBA_GameBackendClientSubsystem* BackendClient = GetBackendClient();
	UDBA_GameBackendPlayerService* PlayerService = BackendClient ? BackendClient->GetPlayerService() : nullptr;
	if (!PlayerService)
	{
		OnComplete.ExecuteIfBound(false, TArray<FDBACharacterSummary>());
		return;
	}

	const uint64 Generation = RequestGeneration;
	PlayerService->GetCharactersAsync([WeakThis = TWeakObjectPtr<UDBAOnlineAccountService>(this), Generation, OnComplete](bool bSuccess, const FString& ErrorMessage, const FString& RawResponse)
	{
		UDBAOnlineAccountService* Service = WeakThis.Get();
		if (!Service || !Service->IsRequestCurrent(Generation))
		{
			return;
		}
		if (!bSuccess)
		{
			Service->LogSubsystemWarning(FString::Printf(TEXT("获取角色列表失败：%s"), *ErrorMessage));
			OnComplete.ExecuteIfBound(false, TArray<FDBACharacterSummary>());
			return;
		}

		TArray<FDBACharacterSummary> Characters;
		FString ParseError;
		if (!FDBAOnlineAccountJson::ParseCharacterListResponse(RawResponse, Characters, ParseError))
		{
			Service->LogSubsystemWarning(FString::Printf(TEXT("解析角色列表失败：%s"), *ParseError));
			OnComplete.ExecuteIfBound(false, TArray<FDBACharacterSummary>());
			return;
		}

		Service->SaveOnlineAccountState(&Characters);
		OnComplete.ExecuteIfBound(true, Characters);
	});
}

void UDBAOnlineAccountService::CreateCharacter(const FDBACharacterCreateRequest& RequestData, FDBAOnCharacterCreated OnComplete)
{
	FString ValidationError;
	if (!ValidateCharacterName(RequestData.CharacterName, ValidationError))
	{
		FDBACharacterCreateResponse Response;
		Response.ErrorMessage = ValidationError;
		OnComplete.ExecuteIfBound(Response);
		return;
	}

	UDBA_GameBackendClientSubsystem* BackendClient = GetBackendClient();
	UDBA_GameBackendPlayerService* PlayerService = BackendClient ? BackendClient->GetPlayerService() : nullptr;
	if (!PlayerService)
	{
		FDBACharacterCreateResponse Response;
		Response.ErrorMessage = TEXT("创建角色失败：后端玩家服务不可用。");
		OnComplete.ExecuteIfBound(Response);
		return;
	}

	const FString CharacterJson = FDBAOnlineAccountJson::BuildCreateCharacterRequest(RequestData);
	const uint64 Generation = RequestGeneration;
	PlayerService->CreateCharacterAsync(CharacterJson,
		[WeakThis = TWeakObjectPtr<UDBAOnlineAccountService>(this), Generation, OnComplete](bool bSuccess, const FString& ErrorMessage, const FString& RawResponse)
		{
			UDBAOnlineAccountService* Service = WeakThis.Get();
			if (!Service || !Service->IsRequestCurrent(Generation))
			{
				return;
			}

			FDBACharacterCreateResponse Response;
			if (!bSuccess)
			{
				Response.ErrorMessage = FString::Printf(TEXT("创建角色失败：%s"), *ErrorMessage);
				OnComplete.ExecuteIfBound(Response);
				return;
			}

			FString ParseError;
			if (!FDBAOnlineAccountJson::ParseCreateCharacterResponse(RawResponse, Response, ParseError))
			{
				Response.ErrorMessage = FString::Printf(TEXT("创建角色响应解析失败：%s"), *ParseError);
			}
			if (Response.bSuccess)
			{
				Service->SaveOnlineAccountState();
			}
			OnComplete.ExecuteIfBound(Response);
		});
}

void UDBAOnlineAccountService::SelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete)
{
	if (!EnsureGameThread(TEXT("SelectCharacter")) || !CharacterId.IsValid())
	{
		OnComplete.ExecuteIfBound(FDBACharacterId());
		return;
	}

	UDBA_GameBackendClientSubsystem* BackendClient = GetBackendClient();
	UDBA_GameBackendPlayerService* PlayerService = BackendClient ? BackendClient->GetPlayerService() : nullptr;
	if (!PlayerService)
	{
		OnComplete.ExecuteIfBound(FDBACharacterId());
		return;
	}

	const uint64 Generation = RequestGeneration;
	PlayerService->SelectCharacterAsync(CharacterId.ToString(),
		[WeakThis = TWeakObjectPtr<UDBAOnlineAccountService>(this), Generation, OnComplete](bool bSuccess, const FString& ErrorMessage, const FString& RawResponse)
		{
			UDBAOnlineAccountService* Service = WeakThis.Get();
			if (!Service || !Service->IsRequestCurrent(Generation))
			{
				return;
			}
			if (!bSuccess)
			{
				Service->LogSubsystemWarning(FString::Printf(TEXT("选择角色失败：%s"), *ErrorMessage));
				OnComplete.ExecuteIfBound(FDBACharacterId());
				return;
			}

			FDBACharacterId SelectedId;
			FString ParseError;
			if (!FDBAOnlineAccountJson::ParseSelectCharacterResponse(RawResponse, SelectedId, ParseError) || !SelectedId.IsValid())
			{
				Service->LogSubsystemWarning(FString::Printf(TEXT("解析选择角色响应失败：%s"), *ParseError));
				OnComplete.ExecuteIfBound(FDBACharacterId());
				return;
			}

			Service->CurrentCharacterId = SelectedId;
			Service->SaveOnlineAccountState();
			OnComplete.ExecuteIfBound(SelectedId);
		});
}
