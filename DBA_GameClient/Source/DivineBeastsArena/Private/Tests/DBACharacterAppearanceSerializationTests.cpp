// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterAppearanceSerializationTest,
	"DBA.Character.Appearance.SerializationRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterAppearanceSerializationTest::RunTest(const FString& Parameters)
{
	FDBACharacterAppearance Source;
	Source.GenderId = TEXT("gender.standard");
	Source.BodyTypeId = TEXT("body.athletic");
	Source.FaceId = TEXT("face.default");
	Source.HairId = TEXT("hair.short");
	Source.HairColorId = TEXT("hair_color.black");
	Source.SkinColorId = TEXT("skin.neutral");
	Source.EyeColorId = TEXT("eye.amber");
	Source.MarkingId = TEXT("marking.none");
	Source.HornId = TEXT("horn.none");
	Source.EarId = TEXT("ear.default");
	Source.TailId = TEXT("tail.default");
	Source.EquipmentVisualIds = { TEXT("equipment.cloth.01"), TEXT("equipment.boot.01") };
	Source.WeaponVisualId = TEXT("weapon.staff.01");
	Source.SkinId = TEXT("skin.base");

	FString Json;
	TestTrue(TEXT("外观稳定 ID 应可序列化为 JSON"), DBACharacterAppearanceSerialization::ToJson(Source, Json));
	TestFalse(TEXT("外观 JSON 不得包含客户端资产路径"), Json.Contains(TEXT("/Game/")) || Json.Contains(TEXT(".uasset")));

	FDBACharacterAppearance Restored;
	TestTrue(TEXT("外观 JSON 应可反序列化"), DBACharacterAppearanceSerialization::FromJson(Json, Restored));
	TestTrue(TEXT("外观序列化往返后应保持所有稳定 ID"), Source == Restored);
	return true;
}

#endif
