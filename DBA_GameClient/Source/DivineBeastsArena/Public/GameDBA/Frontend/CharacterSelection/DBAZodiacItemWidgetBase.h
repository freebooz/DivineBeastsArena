// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateZodiacViewModel.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBAZodiacItemWidgetBase.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnZodiacCreateItemClicked, EDBAZodiac, Zodiac);

/** WBP_DBA_ZodiacItem 的轻量 C++ 父类：只显示数据并发出点击意图，不直接访问 Draft 或 Preview。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBAZodiacItemWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	/** 容器在 ViewModel 变化时调用；头像仍为软引用，加载策略由 Blueprint 表现层决定。 */
	void ApplyItem(const FDBAZodiacCreateListItem& InItem);

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterCreate|Zodiac")
	FDBAOnZodiacCreateItemClicked OnZodiacClicked;

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate|Zodiac")
	void BP_OnItemApplied(const FDBAZodiacCreateListItem& InItem);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate|Zodiac")
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Zodiac")
	FDBAZodiacCreateListItem Item;
};
