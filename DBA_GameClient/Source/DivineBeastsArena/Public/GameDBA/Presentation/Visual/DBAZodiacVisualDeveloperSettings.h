// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：十二生肖角色外观策略配置（占位染色 vs 独立模型）。
- 阅读重点：bUseTintedPlaceholderMesh 为 true 时，所有生肖共用 PlaceholderSkeletalMesh，仅通过染色区分。
- 修改提示：切换为真实模型时将 bUseTintedPlaceholderMesh 设为 false，并确保各生肖 SKM 资产可用。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DBAZodiacVisualDeveloperSettings.generated.h"

class UDataTable;
class UAnimationAsset;
class UMaterialInterface;
class USkeletalMesh;
class UStaticMesh;
class UDBAZodiacCharacterRegistry;

/**
 * UDBAZodiacVisualDeveloperSettings
 * 十二生肖视觉策略配置
 *
 * 阶段策略：
 * - 当前（默认）：共用占位骨骼网格 + RuntimeTint 材质染色区分 12 生肖
 * - 后期：关闭 bUseTintedPlaceholderMesh，改用各生肖独立 SKM_DBA_Zodiac_* 模型
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA 生肖外观配置"))
class DIVINEBEASTSARENA_API UDBAZodiacVisualDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDBAZodiacVisualDeveloperSettings();

	/** 玩家选角、创建和大厅角色的唯一角色注册表入口。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Registry")
	TSoftObjectPtr<UDBAZodiacCharacterRegistry> ZodiacCharacterRegistry;

	/** 为 true 时：大厅/选角/预览统一使用占位模型，仅通过材质颜色区分生肖 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Visual")
	bool bUseTintedPlaceholderMesh = true;

	/** 占位用骨骼网格（当前默认 Rosales 开发角色） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Visual|Placeholder", meta = (AllowedClasses = "/Script/Engine.SkeletalMesh"))
	TSoftObjectPtr<USkeletalMesh> PlaceholderSkeletalMesh;

	/** 占位染色母材质（需支持 Tint/BaseColor 等向量参数） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Visual|Placeholder", meta = (AllowedClasses = "/Script/Engine.MaterialInterface"))
	TSoftObjectPtr<UMaterialInterface> PlaceholderTintMaterial;

	/** 十二生肖染色表（行名 = Rat/Ox/…，结构体 FDBAZodiacPlaceholderTintRow） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Visual|Placeholder", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> ZodiacPlaceholderTintTable;

	/** 大厅训练怪物骨骼网格；未配置时复用 PlaceholderSkeletalMesh。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Lobby|TrainingMonster", meta = (AllowedClasses = "/Script/Engine.SkeletalMesh"))
	TSoftObjectPtr<USkeletalMesh> LobbyTrainingMonsterMesh;

	/** 大厅训练怪物单节点待机动画。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Lobby|TrainingMonster", meta = (AllowedClasses = "/Script/Engine.AnimationAsset"))
	TSoftObjectPtr<UAnimationAsset> LobbyTrainingMonsterIdleAnimation;

	/** 大厅训练怪物单节点行走动画。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Lobby|TrainingMonster", meta = (AllowedClasses = "/Script/Engine.AnimationAsset"))
	TSoftObjectPtr<UAnimationAsset> LobbyTrainingMonsterWalkAnimation;

	/** 大厅训练怪物与选中环共用的染色材质；未配置时复用 PlaceholderTintMaterial。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Lobby|TrainingMonster", meta = (AllowedClasses = "/Script/Engine.MaterialInterface"))
	TSoftObjectPtr<UMaterialInterface> LobbyTrainingMonsterTintMaterial;

	/** 大厅训练怪物选中环网格。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Lobby|TrainingMonster", meta = (AllowedClasses = "/Script/Engine.StaticMesh"))
	TSoftObjectPtr<UStaticMesh> LobbyTrainingMonsterSelectionRingMesh;
};
