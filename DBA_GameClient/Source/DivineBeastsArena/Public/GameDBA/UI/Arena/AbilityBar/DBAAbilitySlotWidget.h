// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 技能槽Widget

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBAAbilitySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
struct FDBAPlayableSkillRuntimeSpec;

/**
 * FDBAAbilityInfo
 * 技能信息结构
 */
USTRUCT(BlueprintType)
struct FDBAAbilityInfo
{
	GENERATED_BODY()

public:
	/** 技能名称 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText AbilityName;

	/** 技能图标 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/** 快捷键 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FKey Hotkey;

	/** 冷却时间 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Cooldown = 0.0f;

	/** 当前冷却剩余 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CurrentCooldown = 0.0f;

	/** 是否启用 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bEnabled = true;
};

/**
 * UDBAAbilitySlotWidget
 * 技能槽Widget
 * 显示单个技能的图标、冷却、快捷键
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAAbilitySlotWidget : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAAbilitySlotWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

	/** 技能图标 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> SkillIcon;

	/** 冷却遮罩 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> CooldownOverlay;

	/** 冷却文字 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CooldownText;

	/** 快捷键文本 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HotkeyText;

	/** 技能可用时的高亮 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> AvailableHighlight;

public:
	/** 设置技能信息 */
	UFUNCTION(BlueprintCallable, Category = "UI|AbilitySlot")
	void SetAbilityInfo(const FDBAAbilityInfo& Info);

	UFUNCTION(BlueprintCallable, Category = "UI|AbilitySlot")
	void SetAbilityFromPlayableSkill(const FDBAPlayableSkillRuntimeSpec& SkillSpec, FKey InHotkey);

	/** 设置冷却状态 */
	UFUNCTION(BlueprintCallable, Category = "UI|AbilitySlot")
	void SetCooldown(float RemainingTime, float TotalTime);

	/** 设置是否可用 */
	UFUNCTION(BlueprintCallable, Category = "UI|AbilitySlot")
	void SetAvailable(bool bAvailable);

	/** 获取技能信息 */
	UFUNCTION(BlueprintCallable, Category = "UI|AbilitySlot")
	const FDBAAbilityInfo& GetAbilityInfo() const { return AbilityInfo; }

protected:
	/** 技能信息 */
	UPROPERTY(Transient)
	FDBAAbilityInfo AbilityInfo;

	/** 更新冷却显示 */
	void UpdateCooldownDisplay();

	/** 更新图标显示 */
	void UpdateIconDisplay();

	/** 更新快捷键显示 */
	void UpdateHotkeyDisplay();
};
