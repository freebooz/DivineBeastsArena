// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBAFixedSkillBuildPreviewWidgetBase.generated.h"

class UDBACharacterCreateWidgetController;
class UDBACharacterCreateElementViewModel;

/** WBP_DBA_FixedSkillBuildPreview 的父类；只展示规则锁定的技能顺序。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBAFixedSkillBuildPreviewWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|FixedBuild")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController);

protected:
	virtual void NativeDestruct() override;
	UFUNCTION()
	void HandleViewModelChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate|FixedBuild")
	void BP_OnFixedSkillBuildChanged(const FDBAFixedSkillBuildPreviewModel& Preview);

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;
	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateElementViewModel> ViewModel;
};
