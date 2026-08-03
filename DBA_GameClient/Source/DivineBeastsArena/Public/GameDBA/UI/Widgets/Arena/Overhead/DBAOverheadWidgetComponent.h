// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

	/** 应用头顶Widget显示配置 */
	void ApplyWidgetConfig();

	/** 更新Widget位置 */
	void UpdateWidgetPosition();

	/** 获取Owning Actor的边界 */
	FVector GetOwnerBoundingBoxCenter() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> OverheadWidget;

	UPROPERTY(Transient)
	float CachedHealthPercent = 1.0f;

	UPROPERTY(Transient)
	FText CachedCharacterName;

	UPROPERTY(Transient)
	bool bCachedOverheadVisible = true;
};
