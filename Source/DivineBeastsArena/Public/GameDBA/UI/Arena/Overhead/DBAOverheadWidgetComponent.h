// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "Components/SceneComponent.h"
#include "DBAOverheadWidgetComponent.generated.h"

class UUserWidget;

/**
 * UDBAOverheadWidgetComponent
 * 单位头顶Widget组件
 * 负责在单位头顶显示血条、名字等信息
 */
UCLASS(Abstract, Blueprintable, BlueprintType, meta = (DisplayName = "DBA Overhead Widget Component"))
class DIVINEBEASTSARENA_API UDBAOverheadWidgetComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDBAOverheadWidgetComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 头顶Widget类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Overhead")
	TSubclassOf<UUserWidget> OverheadWidgetClass;

	/** 显示血条 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Overhead")
	bool bShowHealthBar = true;

	/** 显示名字 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Overhead")
	bool bShowName = true;

	/** 血条颜色（可配置） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Overhead")
	FLinearColor HealthBarColor = FLinearColor::Red;

	/** 背景颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Overhead")
	FLinearColor BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.6f);

	/** 血条高度偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Overhead")
	float HealthBarHeightOffset = 120.0f;

public:
	/** 设置血条百分比 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Overhead")
	void SetHealthBarPercent(float Percent);

	/** 设置角色名字 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Overhead")
	void SetCharacterName(const FText& Name);

	/** 设置是否显示 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Overhead")
	void SetOverheadVisible(bool bShouldBeVisible);

protected:
	/** 创建头顶Widget */
	void CreateOverheadWidget();

	/** 更新Widget位置 */
	void UpdateWidgetPosition();

	/** 获取Owning Actor的边界 */
	FVector GetOwnerBoundingBoxCenter() const;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> OverheadWidget;

	UPROPERTY()
	float CachedHealthPercent = 1.0f;
};