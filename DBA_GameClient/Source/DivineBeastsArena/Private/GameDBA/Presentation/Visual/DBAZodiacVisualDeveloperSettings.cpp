// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Presentation/Visual/DBAZodiacVisualDeveloperSettings.h"

UDBAZodiacVisualDeveloperSettings::UDBAZodiacVisualDeveloperSettings()
{
	bUseTintedPlaceholderMesh = true;
	PlaceholderSkeletalMesh = TSoftObjectPtr<USkeletalMesh>(
		FSoftObjectPath(TEXT("/Game/DBA/Characters/Rosales/Meshes/SK_Rosales.SK_Rosales")));
	PlaceholderTintMaterial = TSoftObjectPtr<UMaterialInterface>(
		FSoftObjectPath(TEXT("/Game/DBA/Materials/M_DBA_RuntimeTint.M_DBA_RuntimeTint")));
	ZodiacPlaceholderTintTable = TSoftObjectPtr<UDataTable>(
		FSoftObjectPath(TEXT("/Game/DBA/Data/Tables/DT_ZodiacPlaceholderTints.DT_ZodiacPlaceholderTints")));
	LobbyTrainingMonsterMesh = TSoftObjectPtr<USkeletalMesh>(
		FSoftObjectPath(TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat")));
	LobbyTrainingMonsterIdleAnimation = TSoftObjectPtr<UAnimationAsset>();
	LobbyTrainingMonsterWalkAnimation = TSoftObjectPtr<UAnimationAsset>();
	LobbyTrainingMonsterTintMaterial = PlaceholderTintMaterial;
	LobbyTrainingMonsterSelectionRingMesh = TSoftObjectPtr<UStaticMesh>(
		FSoftObjectPath(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
}
