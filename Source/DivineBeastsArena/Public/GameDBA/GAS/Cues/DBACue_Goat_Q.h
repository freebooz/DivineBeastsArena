// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 缇奞鎶€鑳?
#pragma once

#include "CoreMinimal.h"
#include "GameDBA/GAS/Cues/DBACue_Base.h"
#include "GameDBA/Data/DBASkillDataRow.h"
#include "DBACue_Goat_Q.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API ADBACue_Goat_Q : public ADBACue_Base
{
	GENERATED_BODY()

public:
	ADBACue_Goat_Q();

	// 褰?Cue 琚Е鍙戞椂璋冪敤
	virtual bool OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);

	// 褰?Cue 鐢熸晥鏃惰皟鐢?(鎸佺画鎬?Cue)
	virtual void OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);

	// Called when this cue is removed.
	virtual void OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);

protected:
	// 浠庢妧鑳芥暟鎹〃鍔犺浇閰嶇疆
	void LoadSkillData();

protected:

	// 鎶€鑳絀D (鐢ㄤ簬鏌ヨ鏁版嵁)
	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	FName SkillId = FName(TEXT("Goat_Q"));

protected:
	virtual FName GetSkillId() const override { return FName(TEXT("Goat_Q")); }
};

