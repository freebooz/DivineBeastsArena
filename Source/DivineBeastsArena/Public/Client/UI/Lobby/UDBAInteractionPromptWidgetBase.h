// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBAInteractionPromptWidgetBase.generated.h"

UENUM(BlueprintType)
enum class EDBAInteractionType : uint8
{
	None UMETA(DisplayName = "无交互"),
	Gate UMETA(DisplayName = "门禁"),
	Portal UMETA(DisplayName = "传送门"),
	NPC UMETA(DisplayName = "NPC对话"),
	TrainingDummy UMETA(DisplayName = "训练假人"),
	Shop UMETA(DisplayName = "商店"),
	QuestItem UMETA(DisplayName = "任务物品")
};

/**
 * DBAInteractionPromptWidgetBase
 *
 * 交互提示 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAInteractionPromptWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAInteractionPromptWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|InteractionPrompt")
	void ShowPrompt(EDBAInteractionType Type, const FText& ObjectName, const FText& PromptText, bool bCanInteract);

	UFUNCTION(BlueprintCallable, Category = "DBA|InteractionPrompt")
	void HidePrompt();

	UFUNCTION(BlueprintCallable, Category = "DBA|InteractionPrompt")
	void UpdateInteractionProgress(float Progress);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|InteractionPrompt", meta = (DisplayName = "On Show Prompt"))
	void BP_OnShowPrompt(EDBAInteractionType Type, const FText& ObjectName, const FText& PromptText, bool bCanInteract);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|InteractionPrompt", meta = (DisplayName = "On Hide Prompt"))
	void BP_OnHidePrompt();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|InteractionPrompt", meta = (DisplayName = "On Update Progress"))
	void BP_OnUpdateProgress(float Progress);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|InteractionPrompt")
	EDBAInteractionType InteractionType;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|InteractionPrompt")
	FText CachedObjectName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|InteractionPrompt")
	FText CachedPromptText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|InteractionPrompt")
	bool CachedCanInteract;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|InteractionPrompt")
	float InteractionProgress;
};
