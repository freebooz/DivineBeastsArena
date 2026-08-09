// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateZodiacViewModel.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBAAppearancePanelWidgetBase.generated.h"

class UDBACharacterCreateWidgetController;
class UDBACharacterCreateZodiacViewModel;

/** WBP_DBA_AppearancePanel 的 C++ 父类：Blueprint 负责布局，候选数据和选择意图保持 C++ 驱动。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBAAppearancePanelWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Appearance")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Appearance")
	bool SelectOption(EDBAAppearanceSlot AppearanceSlot, FName OptionId);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Appearance")
	bool Randomize();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Appearance")
	bool ResetToDefault();

protected:
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleViewModelChanged();

	/** Blueprint 从 Groups 动态生成实际支持的 Face/Hair/Color/Horn/Ear/Tail/BodyType 控件，不显示空组。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate|Appearance")
	void BP_OnAppearanceGroupsChanged(const TArray<FDBAAppearanceOptionGroup>& Groups);

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateZodiacViewModel> ViewModel;
};
