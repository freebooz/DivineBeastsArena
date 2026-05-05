// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能槽Widget

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DBAAbilitySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;

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
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAAbilitySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDBAAbilitySlotWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
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
	UPROPERTY()
	FDBAAbilityInfo AbilityInfo;

	/** 更新冷却显示 */
	void UpdateCooldownDisplay();

	/** 更新图标显示 */
	void UpdateIconDisplay();
};