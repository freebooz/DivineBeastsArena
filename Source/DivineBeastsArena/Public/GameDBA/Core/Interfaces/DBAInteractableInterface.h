// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBAInteractableInterface.generated.h"

/**
 * 基础交互接口
 * 用于世界中可交互对象（传送门、NPC、宝箱等）
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBAInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBAInteractableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 是否可交互
	 * @param Interactor 交互发起者
	 * @return 是否可交互
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool CanInteract(AActor* Interactor) const;
	virtual bool CanInteract_Implementation(AActor* Interactor) const { return true; }

	/**
	 * 执行交互
	 * @param Interactor 交互发起者
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor* Interactor);
	virtual void Interact_Implementation(AActor* Interactor) {}

	/**
	 * 获取交互提示文本
	 * @return 提示文本
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractionPrompt() const;
	virtual FText GetInteractionPrompt_Implementation() const { return FText::FromString(TEXT("交互")); }
};
