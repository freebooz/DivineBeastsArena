// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPreviewActor.h"
#include "GameDBA/Framework/DBAGameModeBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Containers/Set.h"

namespace
{
	bool IsReliableTintMaterialParent(const UMaterialInstanceDynamic* Material)
	{
		if (!Material || !Material->Parent)
		{
			return false;
		}

		const FString ParentPath = Material->Parent->GetPathName();
		return ParentPath.Contains(TEXT("/Engine/BasicShapes/BasicShapeMaterial"))
			|| ParentPath.Contains(TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_"));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterPresentationStageSpecTest,
	"DivineBeastsArena.UI.CharacterPresentation.StageSpec",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterPresentationStageSpecTest::RunTest(const FString& Parameters)
{
	const FDBACharacterPresentationStageSpec Spec = ADBACharacterPresentationActor::GetReferenceStageSpec();

	TestEqual(TEXT("Presentation camera should use the reference FOV"), Spec.CameraFOV, 34.0f);
	TestTrue(TEXT("Presentation camera should frame the full zodiac stage"), Spec.CameraLocation.X >= 500.0f);
	TestTrue(TEXT("Presentation camera should sit above the character center"), Spec.CameraLocation.Z <= 150.0f);
	TestEqual(TEXT("Key light should match the reference stage"), Spec.KeyLightIntensity, 65000.0f);
	TestEqual(TEXT("Rim light should match the reference stage"), Spec.RimLightIntensity, 36000.0f);
	TestEqual(TEXT("Stage should have a broad sanctuary floor"), Spec.GroundScale, FVector(7.5f, 7.5f, 1.0f));
	TestTrue(TEXT("Stage should use atmospheric fog"), Spec.bUseAtmosphericFog);
	TestTrue(TEXT("Stage should use an elevated pedestal"), Spec.bUsePedestal);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterPresentationStageLifecycleTest,
	"DivineBeastsArena.UI.CharacterPresentation.StageLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterPresentationStageLifecycleTest::RunTest(const FString& Parameters)
{
	const FString UniqueWorldName = FString::Printf(
		TEXT("DBACharacterPresentationStageLifecycleTest_%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	UPackage* WorldPackage = CreatePackage(*FString::Printf(TEXT("/Temp/%s"), *UniqueWorldName));
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, FName(*UniqueWorldName), WorldPackage, false);
	TestNotNull(TEXT("Test world should be created"), World);
	if (!World)
	{
		return false;
	}

	ADBACharacterPresentationActor* LevelStage = World->SpawnActor<ADBACharacterPresentationActor>(
		ADBACharacterPresentationActor::StaticClass(),
		FVector(123.0f, 45.0f, 67.0f),
		FRotator::ZeroRotator);
	TestNotNull(TEXT("Existing level stage should spawn"), LevelStage);

	ADBACharacterPresentationActor* ResolvedStage = ADBACharacterPresentationActor::ResolveSharedPresentationStage(World);
	TestEqual(TEXT("Shared stage resolver should reuse the existing level actor"), ResolvedStage, LevelStage);

	ADBACharacterPresentationActor* WidgetReference = ResolvedStage;
	ADBACharacterPresentationActor::ReleaseSharedPresentationStage(WidgetReference);
	TestNull(TEXT("Release should clear only the widget reference"), WidgetReference);
	TestTrue(TEXT("Release should not destroy the shared world stage"), IsValid(LevelStage));

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterPresentationZodiacVisualsTest,
	"DivineBeastsArena.UI.CharacterPresentation.ZodiacVisuals",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterPresentationZodiacVisualsTest::RunTest(const FString& Parameters)
{
	const EDBAZodiac Zodiacs[] = {
		EDBAZodiac::Rat,
		EDBAZodiac::Ox,
		EDBAZodiac::Tiger,
		EDBAZodiac::Rabbit,
		EDBAZodiac::Dragon,
		EDBAZodiac::Snake,
		EDBAZodiac::Horse,
		EDBAZodiac::Goat,
		EDBAZodiac::Monkey,
		EDBAZodiac::Rooster,
		EDBAZodiac::Dog,
		EDBAZodiac::Pig
	};

	TSet<FString> MeshPaths;
	TSet<FString> MaterialPaths;
	TSet<FColor> TintColors;
	for (EDBAZodiac Zodiac : Zodiacs)
	{
		const FString MeshPath = ADBACharacterPresentationActor::GetPreviewMeshPathForZodiac(Zodiac);
		const FString MaterialPath = ADBACharacterPresentationActor::GetPreviewMaterialPathForZodiac(Zodiac);
		const FLinearColor Tint = ADBACharacterPresentationActor::GetPreviewTintForZodiac(Zodiac);

		TestTrue(TEXT("Zodiac mesh path should point at a zodiac mesh"), MeshPath.Contains(TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_")));
		TestTrue(TEXT("Zodiac material path should point at a zodiac material instance"), MaterialPath.Contains(TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_")));
		TestTrue(TEXT("Zodiac tint should be visible"), Tint.GetMax() > 0.55f);

		MeshPaths.Add(MeshPath);
		MaterialPaths.Add(MaterialPath);
		TintColors.Add(Tint.ToFColorSRGB());
	}

	const int32 ZodiacCount = static_cast<int32>(UE_ARRAY_COUNT(Zodiacs));
	TestEqual(TEXT("Every zodiac should have a distinct mesh path"), MeshPaths.Num(), ZodiacCount);
	TestEqual(TEXT("Every zodiac should have a distinct material instance path"), MaterialPaths.Num(), ZodiacCount);
	TestEqual(TEXT("Every zodiac should have a distinct characteristic tint"), TintColors.Num(), ZodiacCount);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterPresentationFacingTest,
	"DivineBeastsArena.UI.CharacterPresentation.FacesPlayer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterPresentationFacingTest::RunTest(const FString& Parameters)
{
	const FString UniqueWorldName = FString::Printf(
		TEXT("DBACharacterPresentationFacingTest_%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	UPackage* WorldPackage = CreatePackage(*FString::Printf(TEXT("/Temp/%s"), *UniqueWorldName));
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, FName(*UniqueWorldName), WorldPackage, false);
	TestNotNull(TEXT("Test world should be created"), World);
	if (!World)
	{
		return false;
	}

	ADBACharacterPresentationActor* PresentationActor = World->SpawnActor<ADBACharacterPresentationActor>();
	TestNotNull(TEXT("Presentation actor should spawn"), PresentationActor);
	if (PresentationActor)
	{
		PresentationActor->SetPreviewZodiac(EDBAZodiac::Tiger);
		USkeletalMeshComponent* PresentationMesh = PresentationActor->FindComponentByClass<USkeletalMeshComponent>();
		TestNotNull(TEXT("Presentation actor should expose a preview mesh component"), PresentationMesh);
		if (PresentationMesh)
		{
			TestEqual(TEXT("Presentation zodiac mesh should face the player camera"), static_cast<double>(PresentationMesh->GetRelativeRotation().Yaw), -90.0);
		}
	}

	ADBACharacterPreviewActor* PreviewActor = World->SpawnActor<ADBACharacterPreviewActor>();
	TestNotNull(TEXT("Preview actor should spawn"), PreviewActor);
	if (PreviewActor)
	{
		PreviewActor->SetPreviewZodiac(EDBAZodiac::Dragon);
		USkeletalMeshComponent* PreviewMesh = PreviewActor->FindComponentByClass<USkeletalMeshComponent>();
		TestNotNull(TEXT("Preview actor should expose a preview mesh component"), PreviewMesh);
		if (PreviewMesh)
		{
			TestEqual(TEXT("Standalone zodiac preview mesh should use the same player-facing rotation"), static_cast<double>(PreviewMesh->GetRelativeRotation().Yaw), -90.0);
		}
	}

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterPresentationMaterialApplicationTest,
	"DivineBeastsArena.UI.CharacterPresentation.MaterialApplication",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterPresentationMaterialApplicationTest::RunTest(const FString& Parameters)
{
	const FString UniqueWorldName = FString::Printf(
		TEXT("DBACharacterPresentationMaterialApplicationTest_%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	UPackage* WorldPackage = CreatePackage(*FString::Printf(TEXT("/Temp/%s"), *UniqueWorldName));
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, FName(*UniqueWorldName), WorldPackage, false);
	TestNotNull(TEXT("Test world should be created"), World);
	if (!World)
	{
		return false;
	}

	ADBACharacterPresentationActor* PresentationActor = World->SpawnActor<ADBACharacterPresentationActor>();
	TestNotNull(TEXT("Presentation actor should spawn"), PresentationActor);
	if (PresentationActor)
	{
		PresentationActor->SetPreviewZodiac(EDBAZodiac::Ox);
		USkeletalMeshComponent* PresentationMesh = PresentationActor->FindComponentByClass<USkeletalMeshComponent>();
		TestNotNull(TEXT("Presentation actor should expose a preview mesh component"), PresentationMesh);
		if (PresentationMesh)
		{
			UMaterialInstanceDynamic* AppliedMaterial = Cast<UMaterialInstanceDynamic>(PresentationMesh->GetMaterial(0));
			TestNotNull(TEXT("Presentation zodiac material should be dynamic so color is applied even on slot 0"), AppliedMaterial);
			if (AppliedMaterial)
			{
				TestTrue(TEXT("Presentation zodiac material should use a reliable colored parent"),
					IsReliableTintMaterialParent(AppliedMaterial));

				const FLinearColor ExpectedTint = ADBACharacterPresentationActor::GetPreviewTintForZodiac(EDBAZodiac::Ox);
				TestEqual(TEXT("Presentation zodiac material Color should match the zodiac tint"), AppliedMaterial->K2_GetVectorParameterValue(TEXT("Color")), ExpectedTint);
				TestEqual(TEXT("Presentation zodiac material BaseColor should match the zodiac tint"), AppliedMaterial->K2_GetVectorParameterValue(TEXT("BaseColor")), ExpectedTint);
				TestEqual(TEXT("Presentation zodiac material Tint should match the zodiac tint"), AppliedMaterial->K2_GetVectorParameterValue(TEXT("Tint")), ExpectedTint);
			}
		}
	}

	ADBACharacterPreviewActor* PreviewActor = World->SpawnActor<ADBACharacterPreviewActor>();
	TestNotNull(TEXT("Standalone preview actor should spawn"), PreviewActor);
	if (PreviewActor)
	{
		const FVector ExpectedActorLocation(240.0f, 120.0f, 12.0f);
		PreviewActor->SetActorLocation(ExpectedActorLocation);
		PreviewActor->SetPreviewZodiac(EDBAZodiac::Tiger);
		USkeletalMeshComponent* PreviewMesh = PreviewActor->FindComponentByClass<USkeletalMeshComponent>();
		TestNotNull(TEXT("Standalone preview actor should expose a preview mesh component"), PreviewMesh);
		if (PreviewMesh)
		{
			UMaterialInstanceDynamic* AppliedMaterial = Cast<UMaterialInstanceDynamic>(PreviewMesh->GetMaterial(0));
			TestNotNull(TEXT("Standalone preview zodiac material should be dynamic so color is applied even on slot 0"), AppliedMaterial);
			if (AppliedMaterial)
			{
				TestTrue(TEXT("Standalone preview zodiac material should use a reliable colored parent"),
					IsReliableTintMaterialParent(AppliedMaterial));

				const FLinearColor TigerTint = ADBACharacterPresentationActor::GetPreviewTintForZodiac(EDBAZodiac::Tiger);
				const FLinearColor OxTint = ADBACharacterPresentationActor::GetPreviewTintForZodiac(EDBAZodiac::Ox);
				TestEqual(TEXT("Standalone preview zodiac material Color should match the zodiac tint"), AppliedMaterial->K2_GetVectorParameterValue(TEXT("Color")), TigerTint);
				TestTrue(TEXT("Different zodiacs should produce clearly different material colors"), !AppliedMaterial->K2_GetVectorParameterValue(TEXT("Color")).Equals(OxTint, 0.01f));
			}
		}
		TestEqual(TEXT("Standalone preview mesh floor offset should not move the actor root"), PreviewActor->GetActorLocation(), ExpectedActorLocation);
	}

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterPresentationUsesLobbyModelTest,
	"DivineBeastsArena.UI.CharacterPresentation.UsesLobbyModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterPresentationUsesLobbyModelTest::RunTest(const FString& Parameters)
{
	const TArray<FString> RatCandidates = ADBACharacterPresentationActor::GetLobbyDisplayMeshCandidatePathsForZodiac(EDBAZodiac::Rat);
	TestTrue(TEXT("Lobby display mesh candidates should prefer the Rosales lobby model"), RatCandidates.Num() > 0 && RatCandidates[0].Contains(TEXT("/Game/DBA/Characters/Rosales/Meshes/SK_Rosales")));

	const FString UniqueWorldName = FString::Printf(
		TEXT("DBACharacterPresentationUsesLobbyModelTest_%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	UPackage* WorldPackage = CreatePackage(*FString::Printf(TEXT("/Temp/%s"), *UniqueWorldName));
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, FName(*UniqueWorldName), WorldPackage, false);
	TestNotNull(TEXT("Test world should be created"), World);
	if (!World)
	{
		return false;
	}

	ADBACharacterPresentationActor* PresentationActor = World->SpawnActor<ADBACharacterPresentationActor>();
	TestNotNull(TEXT("Presentation actor should spawn"), PresentationActor);
	if (PresentationActor)
	{
		PresentationActor->SetPreviewZodiac(EDBAZodiac::Rat);
		USkeletalMeshComponent* PresentationMesh = PresentationActor->FindComponentByClass<USkeletalMeshComponent>();
		TestNotNull(TEXT("Presentation actor should expose a preview mesh component"), PresentationMesh);
		if (PresentationMesh && PresentationMesh->GetSkeletalMeshAsset())
		{
			TestTrue(
				TEXT("Character select/create presentation should use the same Rosales model preferred by the lobby"),
				PresentationMesh->GetSkeletalMeshAsset()->GetPathName().Contains(TEXT("/Game/DBA/Characters/Rosales/Meshes/SK_Rosales")));
			TestEqual(
				TEXT("Character select/create presentation should use animation blueprint mode"),
				PresentationMesh->GetAnimationMode(),
				EAnimationMode::AnimationBlueprint);
			TestTrue(
				TEXT("Character select/create presentation should use the Rosales animation blueprint"),
				PresentationMesh->GetAnimClass() && PresentationMesh->GetAnimClass()->GetPathName().Contains(TEXT("/Game/DBA/Characters/Rosales/AnimationBP/ABP_Rosales")));
		}
	}

	ADBACharacterPreviewActor* PreviewActor = World->SpawnActor<ADBACharacterPreviewActor>();
	TestNotNull(TEXT("Standalone preview actor should spawn"), PreviewActor);
	if (PreviewActor)
	{
		PreviewActor->SetPreviewZodiac(EDBAZodiac::Ox);
		USkeletalMeshComponent* PreviewMesh = PreviewActor->FindComponentByClass<USkeletalMeshComponent>();
		TestNotNull(TEXT("Standalone preview actor should expose a preview mesh component"), PreviewMesh);
		if (PreviewMesh && PreviewMesh->GetSkeletalMeshAsset())
		{
			TestTrue(
				TEXT("Standalone preview should use the same Rosales model preferred by the lobby"),
				PreviewMesh->GetSkeletalMeshAsset()->GetPathName().Contains(TEXT("/Game/DBA/Characters/Rosales/Meshes/SK_Rosales")));
			TestEqual(
				TEXT("Standalone preview should use animation blueprint mode"),
				PreviewMesh->GetAnimationMode(),
				EAnimationMode::AnimationBlueprint);
			TestTrue(
				TEXT("Standalone preview should use the Rosales animation blueprint"),
				PreviewMesh->GetAnimClass() && PreviewMesh->GetAnimClass()->GetPathName().Contains(TEXT("/Game/DBA/Characters/Rosales/AnimationBP/ABP_Rosales")));
		}
	}

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBALobbyDisplaySpecTest,
	"DivineBeastsArena.UI.CharacterPresentation.LobbyDisplaySpec",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBALobbyDisplaySpecTest::RunTest(const FString& Parameters)
{
	TestEqual(
		TEXT("Lobby URL zodiac option should override the join order"),
		ADBAGameModeBase::ResolveLobbyDisplayZodiac(TEXT("DBALobbyZodiac=2"), 0),
		EDBAZodiac::Ox);
	TestEqual(
		TEXT("Lobby URL zodiac option should survive Unreal connection options appended after it"),
		ADBAGameModeBase::ResolveLobbyDisplayZodiac(TEXT("?DBALobbyZodiac=1?Name=Freebooz?SplitscreenCount=1"), 2),
		EDBAZodiac::Rat);
	TestEqual(
		TEXT("First lobby player should default to Rat"),
		ADBAGameModeBase::ResolveLobbyDisplayZodiac(TEXT(""), 0),
		EDBAZodiac::Rat);
	TestEqual(
		TEXT("Second lobby player should default to Ox so two auto clients are visually distinct"),
		ADBAGameModeBase::ResolveLobbyDisplayZodiac(TEXT(""), 1),
		EDBAZodiac::Ox);

	const FTransform FirstTransform = ADBAGameModeBase::GetLobbyDisplayTransform(0);
	const FTransform SecondTransform = ADBAGameModeBase::GetLobbyDisplayTransform(1);
	TestNotEqual(
		TEXT("Lobby display actors should use distinct stand positions"),
		FirstTransform.GetLocation(),
		SecondTransform.GetLocation());
	TestEqual(
		TEXT("Lobby display actor roots should face the LobbyMap camera"),
		static_cast<double>(FirstTransform.Rotator().Yaw),
		180.0);
	TestTrue(
		TEXT("Lobby display actors should be scaled to fit the lobby camera"),
		FirstTransform.GetScale3D().X < 1.0f);
	return true;
}

#endif
