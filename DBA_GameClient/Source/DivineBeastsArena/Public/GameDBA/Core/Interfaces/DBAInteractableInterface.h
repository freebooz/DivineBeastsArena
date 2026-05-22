// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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
