// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameInstanceSubsystem.h"
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
	float DamageValue;

	/** 是否暴击 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCritical;

	/** 元素类型 */
	UPROPERTY(BlueprintReadOnly)
	EDBAElementType Element;
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
class DIVINEBEASTSARENA_API UDBACombatFeedbackSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDBACombatFeedbackSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	/** 分发战斗反馈事件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
	void DispatchCombatEvent(const FDBACombatEventData& EventData);

	/** 监听事件 (Blueprint) */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
	void ListenForEvent(FGameplayTag EventTag, const FOnCombatEvent& Delegate);

	/** 停止监听 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
	void StopListeningForEvent(FGameplayTag EventTag, const FOnCombatEvent& Delegate);

	/** 获取元素对应颜色 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
	static FLinearColor GetElementColor(EDBAElementType Element);

public:
	/** 元素颜色配置表 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Feedback|Config")
	TArray<FDBAElementColorPair> ElementColors;

protected:
	/** 根据GameplayTag获取监听者列表 */
	TArray<FOnCombatEvent*>* GetListenersForTag(FGameplayTag Tag);

	/** 清除所有监听 */
	void ClearAllListeners();

private:
	/** 标签到委托的映射 */
	UPROPERTY()
	TMap<FGameplayTag, TArray<FOnCombatEvent>> EventListeners;

	/** 游戏实例引用 */
	UPROPERTY()
	TWeakObjectPtr<UGameInstance> CachedGameInstance;
};