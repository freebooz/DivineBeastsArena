// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DBAGameInstance.generated.h"

/**
 * DBAGameInstance
 *
 * 游戏实例类
 * 生命周期：整个游戏进程
 * 跨关卡持久化，管理全局子系统
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDBAGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;
};
