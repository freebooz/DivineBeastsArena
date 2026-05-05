// Copyright Freebooz Games, Inc. All Rights Reserved.
// 导航代理配置结构

#pragma once

#include "CoreMinimal.h"
#include "FDBANavigationAgent.generated.h"

/**
 * FDBA NavigationAgent
 * 导航代理配置结构
 * 用于配置NavMesh代理的尺寸和行为
 */
USTRUCT(BlueprintType)
struct FDBA NavigationAgent
{
	GENERATED_BODY()

public:
	/** 代理半径 (NavMesh Agent Radius) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Agent", meta = (UIMin = 10.0, UIMax = 100.0))
	float AgentRadius = 34.0f;

	/** 代理高度 (NavMesh Agent Height) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Agent", meta = (UIMin = 50.0, UIMax = 250.0))
	float AgentHeight = 144.0f;

	/** 可行走地面Z值阈值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Agent", meta = (UIMin = 0.0, UIMax = 1.0))
	float WalkableFloorZ = 0.7f;
};