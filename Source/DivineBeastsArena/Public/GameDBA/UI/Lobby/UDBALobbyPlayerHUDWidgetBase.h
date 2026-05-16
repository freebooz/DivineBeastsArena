// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBALobbyPlayerHUDWidgetBase.generated.h"

class UBorder;
class UCanvasPanel;
class UImage;
class UTextBlock;

/**
 * 大厅玩家HUD基类（可被蓝图继承）
 * - 左上：玩家头像/名称/阵营信息
 * - 下方：4技能栏
 * - 右上：小地图（玩家与队友点位）
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBALobbyPlayerHUDWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBALobbyPlayerHUDWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|HUD")
	void RefreshFromCurrentCharacterData();

protected:
	void BuildDefaultLayoutIfNeeded();
	void BuildTopLeftAvatarPanel(UCanvasPanel* RootCanvas);
	void BuildBottomSkillBar(UCanvasPanel* RootCanvas);
	void BuildTopRightMinimap(UCanvasPanel* RootCanvas);
	void ApplyResponsiveLayout(const FVector2D& ViewportSize);

	void UpdateMinimap();
	void ApplySkillLabels(const TArray<FText>& SkillLabels);
	void ApplySkillHotkeys(const TArray<FText>& SkillHotkeys);
	void ResolveSkillLabelsForSummary(const FDBACharacterSummary& Summary, TArray<FText>& OutSkillLabels) const;
	void ResolveSkillHotkeysForSummary(const FDBACharacterSummary& Summary, TArray<FText>& OutSkillHotkeys) const;
	bool ResolveCurrentCharacterSummary(FDBACharacterSummary& OutSummary) const;

	static FText ZodiacToShortText(EDBAZodiac Zodiac);
	static FText ElementToShortText(EDBAElement Element);
	static FLinearColor ZodiacToColor(EDBAZodiac Zodiac);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|HUD|Layout")
	FVector2D AvatarPanelSize = FVector2D(300.0f, 108.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|HUD|Layout")
	FVector2D SkillSlotSize = FVector2D(124.0f, 70.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|HUD|Layout")
	FVector2D MinimapSize = FVector2D(220.0f, 220.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|HUD|Minimap")
	float MinimapWorldRange = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|HUD|Minimap")
	int32 MaxMinimapTrackedPlayers = 8;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvasPanel;

	UPROPERTY(Transient)
	TObjectPtr<class UCanvasPanelSlot> AvatarRootSlot;

	UPROPERTY(Transient)
	TObjectPtr<class UCanvasPanelSlot> SkillBarRootSlot;

	UPROPERTY(Transient)
	TObjectPtr<class UCanvasPanelSlot> MinimapRootSlot;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> AvatarRootBorder;

	UPROPERTY(Transient)
	TObjectPtr<UImage> AvatarImage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AvatarNameText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AvatarMetaText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> SkillSlotBorders;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> SkillNameTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> SkillHotkeyTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> SkillCooldownTexts;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> MinimapRootBorder;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> MinimapDotCanvas;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> MinimapDots;

	UPROPERTY(Transient)
	FVector MinimapOriginWorld = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bMinimapOriginInitialized = false;

	UPROPERTY(Transient)
	FVector2D LastResponsiveViewport = FVector2D::ZeroVector;
};
