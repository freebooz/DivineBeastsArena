// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountTypes.h"

class FDBAOnlineAccountJson
{
public:
	static FString BuildLoginRequest(const FDBALoginRequest& Request);
	static FString BuildCreateCharacterRequest(const FDBACharacterCreateRequest& Request);

	static bool ParseLoginResponse(const FString& Json, FDBALoginResponse& OutResponse, FString& OutError);
	static bool ParseCharacterListResponse(const FString& Json, TArray<FDBACharacterSummary>& OutCharacters, FString& OutError);
	static bool ParseCreateCharacterResponse(const FString& Json, FDBACharacterCreateResponse& OutResponse, FString& OutError);

	static EDBALoginType ParseLoginType(const FString& Value);
	static EDBAAccountStatus ParseAccountStatus(const FString& Value);
	static EDBAZodiac ParseZodiac(const FString& Value);
	static EDBAElement ParseElement(const FString& Value);
	static EDBAFiveCamp ParseFiveCamp(const FString& Value);

	static FString ToString(EDBALoginType Value);
	static FString ToString(EDBAAccountStatus Value);
	static FString ToString(EDBAZodiac Value);
	static FString ToString(EDBAElement Value);
	static FString ToString(EDBAFiveCamp Value);
};
