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
#include "GameDBA/Gameplay/Abilities/DBAZodiacAbilityBase.h"
#include "DBAResonanceAbilityBase.generated.h"

/**
 * 自然元素共鸣被动基类 (Resonance)
 * 自动授予，不绑定输入，根据 ResonanceLevel 提供 Buff
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAResonanceAbilityBase : public UDBAMobaGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UDBAResonanceAbilityBase();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * 应用共鸣效果
	 * - Lv.1: 控制时间 +0.25 秒，护盾值 +5%
	 * - Lv.2: 控制时间 +0.50 秒，护盾值 +10%
	 * ...
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Resonance")
	void ApplyResonanceEffect(int32 CurrentResonanceLevel);
};
