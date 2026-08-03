// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBAInteractionPromptWidgetBase.generated.h"

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
class DIVINEBEASTSARENA_API UDBAInteractionPromptWidgetBase : public UDBAMobaUserWidgetBase
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
