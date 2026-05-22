// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 怪物仇恨信息结构

#pragma once

#include "CoreMinimal.h"
#include "FAggroInfo.generated.h"

/**
 * FAggroInfo
 * 怪物仇恨信息结构
 * 用于记录每个目标的仇恨值
 */
USTRUCT(BlueprintType)
struct FAggroInfo
{
	GENERATED_BODY()

public:
	/** 目标Actor */
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> Target;

	/** 仇恨值 */
	UPROPERTY(Transient)
	float Threat = 0.0f;

	/** 最后仇恨时间戳 */
	UPROPERTY(Transient)
	float LastThreatTime = 0.0f;

public:
	FAggroInfo() {}

	FAggroInfo(AActor* InTarget, float InThreat, float InWorldTime)
		: Target(InTarget)
		, Threat(InThreat)
		, LastThreatTime(InWorldTime)
	{}

	bool IsValid() const
	{
		return Target.IsValid() && Threat > 0.0f;
	}

	void UpdateThreat(float NewThreat, float CurrentWorldTime)
	{
		Threat = NewThreat;
		LastThreatTime = CurrentWorldTime;
	}

	void AddThreat(float Delta, float CurrentWorldTime)
	{
		Threat += Delta;
		LastThreatTime = CurrentWorldTime;
	}
};