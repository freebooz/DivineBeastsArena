// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "UDBACharacterCreateFlowWidgetBase.generated.h"

/**
 * UDBACharacterCreateFlowWidgetBase
 * 角色创建流程Widget基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterCreateFlowWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBACharacterCreateFlowWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

public:
	/** 设置角色名称 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void SetCharacterName(const FString& Name);

	/** 设置生肖 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void SetZodiac(EDBAZodiac Zodiac);

	/** 设置元素 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void SetElement(EDBAElement Element);

	/** 设置阵营 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void SetFiveCamp(EDBAFiveCamp FiveCamp);

	/** 提交创建 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void Submit();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate", meta = (DisplayName = "On Validation Changed"))
	void BP_OnValidationChanged(bool bInIsValid, const FText& ValidationMessage);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	FString CharacterName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	EDBAZodiac SelectedZodiac = EDBAZodiac::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	EDBAElement SelectedElement = EDBAElement::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	EDBAFiveCamp SelectedFiveCamp = EDBAFiveCamp::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	bool bIsCreateValid = false;
};
