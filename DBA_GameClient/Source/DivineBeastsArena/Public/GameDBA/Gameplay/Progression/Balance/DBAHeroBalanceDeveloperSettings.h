// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明英雄数值平衡 DataTable 的项目级开发配置入口。
- 修改提示：这里只保存软引用配置，不承载战斗逻辑或运行数值。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DBAHeroBalanceDeveloperSettings.generated.h"

class UDataTable;

/**
 * UDBAHeroBalanceDeveloperSettings
 *
 * 英雄数值平衡 DataTable 的项目级配置入口。
 *
 * 通过 DefaultGame.ini 配置：
 * [/Script/DivineBeastsArena.DBAHeroBalanceDeveloperSettings]
 * DefaultHeroBalanceTable=/Game/DBA/Data/Tables/DT_HeroBalance.DT_HeroBalance
 *
 * 运行时由 UDBAHeroBalanceSubsystem 异步加载并缓存。
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA 英雄数值平衡设置"))
class DIVINEBEASTSARENA_API UDBAHeroBalanceDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 项目级默认英雄数值平衡 DataTable。Subsystem 初始化时异步加载。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|Hero Balance")
	TSoftObjectPtr<UDataTable> DefaultHeroBalanceTable;
};
