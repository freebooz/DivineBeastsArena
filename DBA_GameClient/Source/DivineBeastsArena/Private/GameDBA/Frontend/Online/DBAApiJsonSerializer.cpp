// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Online/DBAApiJsonSerializer.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool FDBAApiJsonSerializer::SerializeObject(const TSharedPtr<FJsonObject>& Object, FString& OutJson)
{
	OutJson.Reset();
	if (!Object.IsValid())
	{
		return false;
	}

	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	return FJsonSerializer::Serialize(Object.ToSharedRef(), Writer);
}

bool FDBAApiJsonSerializer::DeserializeObject(const FString& Json, TSharedPtr<FJsonObject>& OutObject)
{
	OutObject.Reset();
	if (Json.IsEmpty())
	{
		return false;
	}

	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
}
