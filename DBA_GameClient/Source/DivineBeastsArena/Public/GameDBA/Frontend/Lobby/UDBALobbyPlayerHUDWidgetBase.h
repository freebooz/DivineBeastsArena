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
#include "GameCore/Networking/Account/DBAAccountTypes.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBALobbyPlayerHUDWidgetBase.generated.h"

class UBorder;
class UButton;
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
	void ResolveBoundWidgetsFromWidgetTree();
	void BindSkillButtonDelegates();
	void ApplyResponsiveLayout(const FVector2D& ViewportSize);
	void EnforceLobbyHudLayoutLimits();

	void UpdateMinimap();
	void UpdateSkillCooldownDisplay(float DeltaTime);
	void ApplySkillLabels(const TArray<FText>& SkillLabels);
	void ApplySkillHotkeys(const TArray<FText>& SkillHotkeys);
	void ResolveSkillLabelsForSummary(const FDBACharacterSummary& Summary, TArray<FText>& OutSkillLabels) const;
	void ResolveSkillHotkeysForSummary(const FDBACharacterSummary& Summary, TArray<FText>& OutSkillHotkeys) const;
	bool ResolveCurrentCharacterSummary(FDBACharacterSummary& OutSummary) const;
	void HandleSkillButtonClicked(int32 SkillSlot);

	UFUNCTION()
	void HandleSkill01ButtonClicked();

	UFUNCTION()
	void HandleSkill02ButtonClicked();

	UFUNCTION()
	void HandleSkill03ButtonClicked();

	UFUNCTION()
	void HandleSkill04ButtonClicked();

	UFUNCTION()
	void HandleUltimateButtonClicked();

	static FText ZodiacToShortText(EDBAZodiac Zodiac);
	static FText ElementToShortText(EDBAElement Element);
	static FLinearColor ZodiacToColor(EDBAZodiac Zodiac);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|HUD|Layout")
	FVector2D AvatarPanelSize = FVector2D(238.0f, 76.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|HUD|Layout")
	FVector2D SkillSlotSize = FVector2D(42.0f, 42.0f);

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
	TObjectPtr<UImage> AvatarBackdropImage;

	UPROPERTY(Transient)
	TObjectPtr<UImage> AvatarFrameImage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AvatarLevelText;

	UPROPERTY(Transient)
	TObjectPtr<UImage> AvatarHealthBarImage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AvatarHealthText;

	UPROPERTY(Transient)
	TObjectPtr<UImage> AvatarManaBarImage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AvatarManaText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AvatarNameText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AvatarMetaText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> SkillSlotBorders;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> SkillSlotButtons;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> SkillBarRootBorder;

	UPROPERTY(Transient)
	TObjectPtr<UImage> SkillBarBackdropImage;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> SkillSlotBackdropImages;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> SkillCooldownOverlayImages;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> SkillReadyGlowImages;

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
	TObjectPtr<UImage> MinimapFrameImage;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> MinimapDots;

	UPROPERTY(Transient)
	FVector MinimapOriginWorld = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bMinimapOriginInitialized = false;

	UPROPERTY(Transient)
	FVector2D LastResponsiveViewport = FVector2D::ZeroVector;

	UPROPERTY(Transient)
	TArray<float> LastObservedSkillCooldowns;

	UPROPERTY(Transient)
	TArray<float> SkillReadyPulseTimes;
};
