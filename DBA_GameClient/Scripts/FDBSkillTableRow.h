// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 技能数据表结构体 (由 Python 脚本自动生成)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FDBSkillTableRow.generated.h"

/**
 * FDBSkillTableRow
 * 技能数据表行结构
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBSkillTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString SkillID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString SkillName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Cooldown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float EnergyCost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CastRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString EffectType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString ZodiacType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString CharacterName;
};

// 在 C++ 中使用:
// #include "GameDBA/GAS/DataTables/FDBSkillTableRow.h"
// UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/DBA/Data/Tables/DT_Skills"));
// FDBSkillTableRow* Row = SkillTable->FindRow<FDBSkillTableRow>(FName("Rat_Passive"), TEXT(""));
