// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Settings/DBAFrontendSettings.h"

UDBAFrontendSettings::UDBAFrontendSettings()
{
	CategoryName = TEXT("DBA");
	SectionName = TEXT("Frontend");
}

FName UDBAFrontendSettings::GetCategoryName() const
{
	return TEXT("DBA");
}
