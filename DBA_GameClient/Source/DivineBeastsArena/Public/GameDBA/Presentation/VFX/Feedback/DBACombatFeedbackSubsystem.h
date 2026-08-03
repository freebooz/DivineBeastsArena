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
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameplayTags.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "DBACombatFeedbackSubsystem.generated.h"

class UDBAEffectTableManager;
class UDBAFloatingDamageComponent;

/**
 * FDBACombatEventData
 * 战斗反馈事件数据
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBACombatEventData
{
	GENERATED_BODY()

	/** 事件标签 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EventTag;

	/** 触发者 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Instigator;

	/** 目标 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Target;

	/** 技能ID */
	UPROPERTY(BlueprintReadOnly)
	FName SkillID;

	/** 伤害值 */
	UPROPERTY(BlueprintReadOnly)
	float DamageValue = 0.0f;

	/** 是否暴击 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCritical = false;

	/** 元素类型 */
	UPROPERTY(BlueprintReadOnly)
	EDBAElementType Element = EDBAElementType::None;
};

/**
 * FOnCombatEventDelegate
 * 战斗事件委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatEvent, const FDBACombatEventData&, EventData);

/**
 * FDBAElementColorPair
 * 元素颜色配置
 */
USTRUCT()
struct FDBAElementColorPair
{
	GENERATED_BODY()

	FDBAElementColorPair() : Element(EDBAElementType::None), Color(FLinearColor::White) {}

	FDBAElementColorPair(EDBAElementType InElement, FLinearColor InColor)
		: Element(InElement), Color(InColor) {}

	UPROPERTY(EditDefaultsOnly)
	EDBAElementType Element;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor Color;
};

/**
 * UDBACombatFeedbackSubsystem
 * 战斗反馈子系统
 * 统一管理技能反馈事件的广播和监听
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBACombatFeedbackSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UDBACombatFeedbackSubsystem();

protected:
	// P1-1 改造：重写项目基类生命周期钩子，替代原生 Initialize/Deinitialize
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;

public:
	/** 分发战斗反馈事件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
	void DispatchCombatEvent(const FDBACombatEventData& EventData);

	/** 监听事件 */
	void ListenForEvent(FGameplayTag EventTag, const FOnCombatEvent& Delegate);

	/** 停止监听 */
	void StopListeningForEvent(FGameplayTag EventTag, const FOnCombatEvent& Delegate);

	/** 获取元素对应颜色 */
	FLinearColor GetElementColor(EDBAElementType Element) const;

public:
	/** 元素颜色配置表 (非Blueprint属性) */
	TArray<FDBAElementColorPair> ElementColors;

protected:
	/** 根据GameplayTag获取监听者列表 */
	TArray<FOnCombatEvent>* GetListenersForTag(FGameplayTag Tag);

	/** 清除所有监听 */
	void ClearAllListeners();

private:
	/** 标签到监听者的映射 (内部使用，无需复制) */
	TMap<FGameplayTag, TArray<FOnCombatEvent>> EventListeners;

	/** 游戏实例引用 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UGameInstance> CachedGameInstance;
};
