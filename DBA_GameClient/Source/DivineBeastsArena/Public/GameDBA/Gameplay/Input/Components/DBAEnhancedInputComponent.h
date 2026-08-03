// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明项目统一的 Enhanced Input 组件基类，所有 DBA PlayerController 的 InputComponent 均应使用此类。
- 修改提示：新增项目级输入辅助方法时，优先放在此类中，避免各 PlayerController 重复实现。
*/

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameCore/Core/DBALogChannels.h"
#include "DBAEnhancedInputComponent.generated.h"

/**
 * UDBAEnhancedInputComponent
 * 项目统一的 Enhanced Input 组件基类
 *
 * 设计依据：
 *   - 项目策略《DBA.Agent.DirectExecution》：输入组件统一基类，避免各 PlayerController 重复 Cast
 *   - 输入 P1-6 遗留项：创建 UDBAEnhancedInputComponent 统一基类
 *
 * 用途：
 *   - 替代直接使用 UEnhancedInputComponent，提供项目级扩展点
 *   - 所有 DBA PlayerController 的 InputComponent 均通过 DefaultInput.ini 配置为此类
 *   - 可在此添加项目级输入辅助方法（如批量绑定、输入状态查询等）
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DIVINEBEASTSARENA_API UDBAEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UDBAEnhancedInputComponent();

	/**
	 * 便捷方法：绑定一个 InputAction 到指定回调，并自动检查 Action 有效性。
	 * 若 Action 无效，输出中文警告日志。
	 */
	template <class UserClass, typename Func>
	void BindActionChecked(
		const UInputAction* Action,
		ETriggerEvent TriggerEvent,
		UserClass* Object,
		Func FuncPtr)
	{
		if (!Action)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[DBAEnhancedInputComponent] 绑定失败：InputAction 为空。请检查输入配置数据资产中的引用是否有效。"));
			return;
		}
		BindAction(Action, TriggerEvent, Object, FuncPtr);
	}
};
