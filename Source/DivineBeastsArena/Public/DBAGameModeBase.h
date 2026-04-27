// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - 基础 GameMode

#pragma once

#include "CoreMinimal.h"
#include "MobaBase/Framework/DBAMobaGameModeBase.h"
#include "DBAGameModeBase.generated.h"

/**
 * DBAGameModeBase
 * 神兽竞技场基础 GameMode
 * 用于验证工程编译与 Dedicated Server 启动
 */
UCLASS()
class DIVINEBEASTSARENA_API ADBAGameModeBase : public ADBAMobaGameModeBase
{
    GENERATED_BODY()

public:
    ADBAGameModeBase();

protected:
    // 游戏开始时调用
    virtual void BeginPlay() override;
};