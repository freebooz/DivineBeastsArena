// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 守卫模型基类

#include "GameDBA/Characters/Guardian/DBAGuardianBase.h"
#include "Components/StaticMeshComponent.h"

#ifndef DBA_GUARDIAN_LOG_CATEGORY
DEFINE_LOG_CATEGORY_STATIC(LogDBAGuardian, Log, All);
#define DBA_GUARDIAN_LOG_CATEGORY LogDBAGuardian
#endif

ADBAGuardianBase::ADBAGuardianBase()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ADBAGuardianBase::BeginPlay()
{
	Super::BeginPlay();

	// P2-2 占位告警：守护者未接入 GAS，仅作骨架占位，不应承载 Gameplay 逻辑。
	UE_LOG(LogDBAGuardian, Warning, TEXT("[守护者] 守护者类 %s 未接入 GAS，当前为占位实现。未来接入请参考类头文件 TODO 注释。"), *GetName());
}

void ADBAGuardianBase::PlayAttackVFX(AActor* Target)
{
	// P2-2 占位实现：未接入实际 VFX 逻辑。
	// TODO(P2): 接入 GameplayCue 或 UDBAEffectPlayer 走统一反馈路径。
	UE_LOG(LogDBAGuardian, Verbose, TEXT("[守护者] PlayAttackVFX 占位调用，目标=%s"), Target ? *Target->GetName() : TEXT("空"));
}

void ADBAGuardianBase::PlayHitVFX(AActor* Attacker)
{
	// P2-2 占位实现：未接入实际 VFX 逻辑。
	// TODO(P2): 接入 GameplayCue 或 UDBAEffectPlayer 走统一反馈路径。
	UE_LOG(LogDBAGuardian, Verbose, TEXT("[守护者] PlayHitVFX 占位调用，攻击者=%s"), Attacker ? *Attacker->GetName() : TEXT("空"));
}