// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendClientSettings.h"

UGameBackendClientSettings::UGameBackendClientSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("GameBackendClient");
}

FName UGameBackendClientSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}
