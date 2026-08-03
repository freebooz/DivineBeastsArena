// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 守卫模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBAGuardianBase.generated.h"

/**
 * ADBAGuardianBase
 *
 * 守护者基类（防御塔/雕像/水晶等可被攻击场景单位）。
 *
 * ===================================================================
 * P2-2 评估结论（2026-07-07）：
 * 本类当前为占位空壳实现，仅保留类继承骨架与字段声明，无 Gameplay 逻辑。
 * 评估决定：暂不接入 GAS，避免对空壳代码增加复杂度。
 *
 * 未来接入 GAS 时应遵循以下设计路径（TODO）：
 *   1. 改为继承 ADBACharacterBase 或 ACharacter + IAbilitySystemInterface，
 *      或保留 AActor 但添加 UAbilitySystemComponent 子对象并实现 IAbilitySystemInterface。
 *   2. 添加 UDBABattleAttributeSet 子对象，迁移 MaxHealth/AttackDamage 到 AttributeSet。
 *   3. 通过 UDBABattleAttributeDefaultsDataAsset 异步加载属性默认值（参考 ADBAPlayerState 链路）。
 *   4. 守护者受伤走 UDBADamageCalculator::ApplyDamageToTargetWithCue 统一 ASC 路径。
 *   5. AI 行为（目标选择、攻击逻辑）迁移到 C++ 组件 UDBAGuardianAIComponent。
 *   6. 守护者参数（攻击范围、伤害、出生位置）通过 UDBAGuardianDataAsset 配置。
 *
 * 当前文件中的 MaxHealth/AttackDamage 字段保留是为了维持蓝图兼容性，
 * 但已标记为 deprecated，新增 Gameplay 逻辑不得直接读写这两个字段。
 * ===================================================================
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAGuardianBase : public AActor
{
	GENERATED_BODY()

public:
	ADBAGuardianBase();

protected:
	virtual void BeginPlay() override;

public:
	/**
	 * 播放攻击特效（占位实现，未接入实际 VFX 逻辑）。
	 * TODO(P2): 接入 GameplayCue 或 UDBAEffectPlayer 走统一反馈路径。
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Guardian")
	void PlayAttackVFX(AActor* Target);

	/**
	 * 播放受击特效（占位实现，未接入实际 VFX 逻辑）。
	 * TODO(P2): 接入 GameplayCue 或 UDBAEffectPlayer 走统一反馈路径。
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Guardian")
	void PlayHitVFX(AActor* Attacker);

protected:
	/** 守护者类型标识（Tower/Statue/Crystal） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName GuardianType = FName(TEXT("None"));

	/**
	 * 最大生命值（占位字段，未接入 GAS）。
	 * @deprecated P2-2 标记：未来接入 GAS 后迁移到 UDBABattleAttributeSet，请勿直接读写。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config", meta = (DeprecatedProperty))
	float MaxHealth = 500.0f;

	/**
	 * 攻击伤害（占位字段，未接入 GAS）。
	 * @deprecated P2-2 标记：未来接入 GAS 后迁移到 UDBABattleAttributeSet，请勿直接读写。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config", meta = (DeprecatedProperty))
	float AttackDamage = 25.0f;
};