// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能数据表结构体 (由 Python 脚本自动生成)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FDBSkillTableRow.generated.h"

/**
 * FDBSkillTableRow
 * 技能数据表行结构
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBSkillTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString SkillID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString SkillName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Cooldown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float EnergyCost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CastRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString EffectType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString ZodiacType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString CharacterName;
};

// 在 C++ 中使用:
// #include "GameDBA/GAS/DataTables/FDBSkillTableRow.h"
// UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/DBA/Data/Tables/DT_Skills"));
// FDBSkillTableRow* Row = SkillTable->FindRow<FDBSkillTableRow>(FName("Rat_Passive"), TEXT(""));
