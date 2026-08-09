// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"

#include "JsonObjectConverter.h"

bool FDBACharacterAppearance::operator==(const FDBACharacterAppearance& Other) const
{
	return GenderId == Other.GenderId
		&& BodyTypeId == Other.BodyTypeId
		&& FaceId == Other.FaceId
		&& HairId == Other.HairId
		&& HairColorId == Other.HairColorId
		&& SkinColorId == Other.SkinColorId
		&& EyeColorId == Other.EyeColorId
		&& MarkingId == Other.MarkingId
		&& HornId == Other.HornId
		&& EarId == Other.EarId
		&& TailId == Other.TailId
		&& EquipmentVisualIds == Other.EquipmentVisualIds
		&& WeaponVisualId == Other.WeaponVisualId
		&& SkinId == Other.SkinId;
}

void FDBACharacterAppearance::GetSelectedOptionIds(TArray<FName>& OutOptionIds) const
{
	OutOptionIds.Reset();
	auto AddIfSet = [&OutOptionIds](const FName OptionId)
	{
		if (!OptionId.IsNone())
		{
			OutOptionIds.AddUnique(OptionId);
		}
	};

	AddIfSet(GenderId);
	AddIfSet(BodyTypeId);
	AddIfSet(FaceId);
	AddIfSet(HairId);
	AddIfSet(HairColorId);
	AddIfSet(SkinColorId);
	AddIfSet(EyeColorId);
	AddIfSet(MarkingId);
	AddIfSet(HornId);
	AddIfSet(EarId);
	AddIfSet(TailId);
	for (const FName EquipmentVisualId : EquipmentVisualIds)
	{
		AddIfSet(EquipmentVisualId);
	}
	AddIfSet(WeaponVisualId);
	AddIfSet(SkinId);
}

bool DBACharacterAppearanceSerialization::ToJson(const FDBACharacterAppearance& Appearance, FString& OutJson)
{
	OutJson.Reset();
	return FJsonObjectConverter::UStructToJsonObjectString(Appearance, OutJson);
}

bool DBACharacterAppearanceSerialization::FromJson(const FString& Json, FDBACharacterAppearance& OutAppearance)
{
	OutAppearance = FDBACharacterAppearance();
	return !Json.IsEmpty() && FJsonObjectConverter::JsonObjectStringToUStruct(Json, &OutAppearance, 0, 0, true);
}
