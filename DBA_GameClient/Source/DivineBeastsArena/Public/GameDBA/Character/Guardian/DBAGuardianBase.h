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
 * DBAGuardianBase
 * 守卫模型基类
 * 提供守卫公共功能
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
	/** 播放攻击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Guardian")
	void PlayAttackVFX(AActor* Target);

	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Guardian")
	void PlayHitVFX(AActor* Attacker);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName GuardianType = FName(TEXT("None"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float MaxHealth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float AttackDamage = 25.0f;
};