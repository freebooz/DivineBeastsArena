// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterPresentationStageSpecTest,
	"DivineBeastsArena.UI.CharacterPresentation.StageSpec",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterPresentationStageSpecTest::RunTest(const FString& Parameters)
{
	const FDBACharacterPresentationStageSpec Spec = ADBACharacterPresentationActor::GetReferenceStageSpec();

	TestEqual(TEXT("Presentation camera should use a cinematic FOV"), Spec.CameraFOV, 38.0f);
	TestTrue(TEXT("Presentation camera should be close enough to frame small zodiac meshes"), Spec.CameraLocation.X <= 240.0f);
	TestTrue(TEXT("Presentation camera should sit near character eye level"), Spec.CameraLocation.Z <= 110.0f);
	TestEqual(TEXT("Key light should be warm and strong"), Spec.KeyLightIntensity, 82000.0f);
	TestEqual(TEXT("Rim light should be present for readable silhouette"), Spec.RimLightIntensity, 42000.0f);
	TestEqual(TEXT("Stage should have a broad sanctuary floor"), Spec.GroundScale, FVector(7.5f, 7.5f, 1.0f));
	TestTrue(TEXT("Stage should use atmospheric fog"), Spec.bUseAtmosphericFog);
	TestTrue(TEXT("Stage should use an elevated pedestal"), Spec.bUsePedestal);
	return true;
}

#endif
