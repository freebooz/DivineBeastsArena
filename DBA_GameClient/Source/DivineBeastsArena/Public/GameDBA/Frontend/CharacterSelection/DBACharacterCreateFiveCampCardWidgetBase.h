// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateFiveCampViewModel.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterCreateFiveCampCardWidgetBase.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnCharacterCreateFiveCampCardClicked, EDBAFiveCamp, FiveCamp);

/** WBP_DBA_FiveCampCard 的 C++ 父类；只转发一次选择意图，不创建页面也不直接写 Draft。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateFiveCampCardWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	void ApplyCard(const FDBACharacterCreateFiveCampCardModel& InCard);

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterCreate|FiveCamp")
	FDBAOnCharacterCreateFiveCampCardClicked OnFiveCampClicked;

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleClicked();

	/** 蓝图只负责使用投影更新文本、图标、选中态与动效。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate|FiveCamp")
	void BP_OnCardApplied(const FDBACharacterCreateFiveCampCardModel& InCard);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate|FiveCamp")
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	FDBACharacterCreateFiveCampCardModel Card;
};
