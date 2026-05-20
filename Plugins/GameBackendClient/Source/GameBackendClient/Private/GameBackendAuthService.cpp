// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendAuthService.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	FString MakePayload(const TFunction<void(TSharedRef<FJsonObject>)>& Fill)
	{
		const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
		Fill(Json);
		FString Body;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
		FJsonSerializer::Serialize(Json, Writer);
		return Body;
	}

	void ExecuteResponseDelegate(const FGameBackendResponseDelegate& Callback, const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.IsSuccessful();
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("Request failed.") : Result.Message);
		Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
	}

	FString ReadStringByKeys(const TSharedPtr<FJsonObject>& Obj, const TArray<FString>& Keys)
	{
		if (!Obj.IsValid())
		{
			return FString();
		}

		for (const FString& Key : Keys)
		{
			FString Value;
			if (Obj->TryGetStringField(Key, Value))
			{
				return Value;
			}
		}
		return FString();
	}

	FString BuildBanMessage(const FString& Code, const FString& Message, const FString& DataJson)
	{
		if (!Code.Contains(TEXT("BAN"), ESearchCase::IgnoreCase))
		{
			return Message;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DataJson);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return Message;
		}

		const FString BanReason = ReadStringByKeys(Root, { TEXT("banReason"), TEXT("reason"), TEXT("ban_reason") });
		const FString UnbanTime = ReadStringByKeys(Root, { TEXT("unbanTimeUtc"), TEXT("unbanTime"), TEXT("unban_at"), TEXT("banExpiresAtUtc") });

		FString Out = Message.IsEmpty() ? TEXT("Account is banned.") : Message;
		if (!BanReason.IsEmpty())
		{
			Out += FString::Printf(TEXT("\nReason: %s"), *BanReason);
		}
		if (!UnbanTime.IsEmpty())
		{
			Out += FString::Printf(TEXT("\nUnban Time (UTC): %s"), *UnbanTime);
		}
		return Out;
	}
}

void UGameBackendAuthService::Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UGameBackendAuthService::DevLogin(const FString& DisplayName, const FGameBackendAuthResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), FString(), FString(), FString());
		return;
	}

	const FString Body = MakePayload([&DisplayName](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("displayName"), DisplayName);
	});

	HttpClient->Post(TEXT("/api/auth/dev-login"), Body, [this, Callback](const FGameBackendHttpResult& Result)
	{
		HandleAuthResponse(Result, Callback);
	}, false);
}

void UGameBackendAuthService::GuestLogin(const FGameBackendGuestLoginRequest& Request, const FGameBackendAuthResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), FString(), FString(), FString());
		return;
	}

	const FString Body = MakePayload([&Request](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("deviceId"), Request.DeviceId);
		Json->SetStringField(TEXT("deviceName"), Request.DeviceName);
		Json->SetStringField(TEXT("platform"), Request.Platform);
	});

	HttpClient->Post(TEXT("/api/auth/guest-login"), Body, [this, Callback](const FGameBackendHttpResult& Result)
	{
		HandleAuthResponse(Result, Callback);
	}, false);
}

void UGameBackendAuthService::SteamLogin(const FString& SteamTicket, const FGameBackendAuthResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), FString(), FString(), FString());
		return;
	}

	const FString Body = MakePayload([&SteamTicket](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("steamTicket"), SteamTicket);
	});

	HttpClient->Post(TEXT("/api/auth/external/steam"), Body, [this, Callback](const FGameBackendHttpResult& Result)
	{
		HandleAuthResponse(Result, Callback);
	}, false);
}

void UGameBackendAuthService::EosLogin(const FString& EosToken, const FGameBackendAuthResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), FString(), FString(), FString());
		return;
	}

	const FString Body = MakePayload([&EosToken](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("eosToken"), EosToken);
	});

	HttpClient->Post(TEXT("/api/auth/external/eos"), Body, [this, Callback](const FGameBackendHttpResult& Result)
	{
		HandleAuthResponse(Result, Callback);
	}, false);
}

void UGameBackendAuthService::RefreshToken(const FGameBackendAuthResponseDelegate& Callback)
{
	if (!HttpClient || !Subsystem.IsValid())
	{
		Callback.ExecuteIfBound(false, TEXT("Auth service unavailable."), FString(), FString(), FString());
		return;
	}

	const FString RefreshTokenValue = Subsystem->GetRefreshToken();
	if (RefreshTokenValue.IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("Refresh token is empty."), FString(), FString(), FString());
		return;
	}

	const FString Body = MakePayload([&RefreshTokenValue](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("refreshToken"), RefreshTokenValue);
	});

	HttpClient->Post(TEXT("/api/auth/refresh"), Body, [this, Callback](const FGameBackendHttpResult& Result)
	{
		HandleAuthResponse(Result, Callback);
	}, false);
}

void UGameBackendAuthService::Logout(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Post(TEXT("/api/auth/logout"), TEXT("{}"), [this, Callback](const FGameBackendHttpResult& Result)
	{
		if (Result.IsSuccessful())
		{
			if (Subsystem.IsValid())
			{
				Subsystem->ClearAuthTokens();
			}
		}
		ExecuteResponseDelegate(Callback, Result);
	});
}

void UGameBackendAuthService::GetMe(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Get(TEXT("/api/auth/me"), [Callback](const FGameBackendHttpResult& Result)
	{
		ExecuteResponseDelegate(Callback, Result);
	});
}

void UGameBackendAuthService::HandleAuthResponse(const FGameBackendHttpResult& Result, const FGameBackendAuthResponseDelegate& Callback)
{
	const bool bSuccess = Result.IsSuccessful();
	if (!bSuccess)
	{
		const FString ErrorMessage = BuildBanMessage(Result.Code, Result.Message, Result.DataJson);
		Callback.ExecuteIfBound(false, ErrorMessage.IsEmpty() ? TEXT("Auth request failed.") : ErrorMessage, FString(), FString(), FString());
		return;
	}

	FGameBackendAuthTokens Tokens;
	if (!TryExtractTokens(Result.DataJson, Tokens))
	{
		Callback.ExecuteIfBound(false, TEXT("Auth response missing token fields."), FString(), FString(), FString());
		return;
	}

	if (Subsystem.IsValid())
	{
		Subsystem->SetAuthTokens(Tokens.AccessToken, Tokens.RefreshToken, Tokens.PlayerId);
	}

	Callback.ExecuteIfBound(true, FString(), Tokens.AccessToken, Tokens.RefreshToken, Tokens.PlayerId);
}

bool UGameBackendAuthService::TryExtractTokens(const FString& DataJson, FGameBackendAuthTokens& OutTokens)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DataJson);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return false;
	}

	const auto ReadField = [&Root](const TArray<FString>& Keys) -> FString
	{
		for (const FString& Key : Keys)
		{
			FString Value;
			if (Root->TryGetStringField(Key, Value))
			{
				return Value;
			}
		}
		return FString();
	};

	OutTokens.AccessToken = ReadField({ TEXT("accessToken"), TEXT("access_token"), TEXT("token") });
	OutTokens.RefreshToken = ReadField({ TEXT("refreshToken"), TEXT("refresh_token") });
	OutTokens.PlayerId = ReadField({ TEXT("playerId"), TEXT("player_id"), TEXT("uid") });

	if (OutTokens.AccessToken.IsEmpty() && Root->HasTypedField<EJson::Object>(TEXT("tokens")))
	{
		const TSharedPtr<FJsonObject> TokensObj = Root->GetObjectField(TEXT("tokens"));
		if (TokensObj.IsValid())
		{
			TokensObj->TryGetStringField(TEXT("accessToken"), OutTokens.AccessToken);
			TokensObj->TryGetStringField(TEXT("refreshToken"), OutTokens.RefreshToken);
		}
	}

	if (OutTokens.PlayerId.IsEmpty() && Root->HasTypedField<EJson::Object>(TEXT("player")))
	{
		const TSharedPtr<FJsonObject> PlayerObj = Root->GetObjectField(TEXT("player"));
		if (PlayerObj.IsValid())
		{
			PlayerObj->TryGetStringField(TEXT("playerId"), OutTokens.PlayerId);
		}
	}

	return !OutTokens.AccessToken.IsEmpty();
}
