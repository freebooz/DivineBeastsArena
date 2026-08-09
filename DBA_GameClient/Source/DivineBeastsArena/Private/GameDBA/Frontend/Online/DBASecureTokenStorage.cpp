// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Online/DBASecureTokenStorage.h"

bool UDBADevelopmentTokenStorage::StoreRefreshToken(const FString& InRefreshToken)
{
	RefreshToken = InRefreshToken;
	return true;
}

bool UDBADevelopmentTokenStorage::LoadRefreshToken(FString& OutRefreshToken) const
{
	OutRefreshToken = RefreshToken;
	return !OutRefreshToken.IsEmpty();
}

void UDBADevelopmentTokenStorage::ClearRefreshToken()
{
	RefreshToken.Empty();
}
