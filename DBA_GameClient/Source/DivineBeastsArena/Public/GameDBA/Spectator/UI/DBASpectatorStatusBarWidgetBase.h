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
#include "GameCore/UI/DBAUserWidgetBase.h"
#include "GameDBA/Spectator/DBAObserverTypes.h"
#include "DBASpectatorStatusBarWidgetBase.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;

/**
 * FDBASpectatorMemberUI
 * 观战状态栏成员UI组件
 */
USTRUCT(BlueprintType)
struct FDBASpectatorMemberUI
{
	GENERATED_BODY()

public:
	/** 成员容器Widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Container;

	/** HP条 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HPBar;

	/** Energy条 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> EnergyBar;

	/** 名字文本 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	/** 活动指示器 (当前观看的目标) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> ActiveIndicator;

	/** 技能冷却数组 (Q, W, E, R) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TArray<TObjectPtr<UWidget>> SkillCooldownWidgets;
};

/**
 * UDBASpectatorStatusBarWidgetBase
 * 观战状态栏Widget
 * 显示当前观看的玩家队伍的所有成员状态
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBASpectatorStatusBarWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBASpectatorStatusBarWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	/** 更新状态栏显示 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|StatusBar")
	void UpdateStatus(const TArray<FDBAObserverViewTarget>& TeamMembers, int32 CurrentIndex);

	/** 更新单个成员状态 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|StatusBar")
	void UpdateMemberStatus(int32 Index, const FDBAObserverViewTarget& Member);

	/** 获取成员UI数组 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|StatusBar")
	const TArray<FDBASpectatorMemberUI>& GetMemberUIs() const { return MemberUIs; }

protected:
	/** 最大支持成员数 */
	UPROPERTY(EditDefaultsOnly, Category = "DBA|Spectator|StatusBar")
	int32 MaxMemberCount;

	/** 动态成员UI数组 */
	UPROPERTY(Transient)
	TArray<FDBASpectatorMemberUI> MemberUIs;
};
