// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 角色统一基础类 - 所有角色与怪物的公共父类

#include "GameDBA/Characters/DBACharacterBase.h"

#include "GameDBA/Core/DBALogChannels.h"
#include "Net/UnrealNetwork.h"

ADBACharacterBase::ADBACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
}

// ==================== IAbilitySystemInterface 实现 ====================

UAbilitySystemComponent* ADBACharacterBase::GetAbilitySystemComponent() const
{
	// 默认返回 nullptr，子类必须重写以提供真实的 ASC
	// 玩家角色子类应从 PlayerState 获取，AI 角色子类应从自身组件获取
	return nullptr;
}

// ==================== 角色通用属性访问 ====================

float ADBACharacterBase::GetCurrentHealth() const
{
	// 默认返回 0，子类应该重写以从 GAS AttributeSet 或自定义字段返回真实值
	return 0.0f;
}

float ADBACharacterBase::GetMaxHealth() const
{
	// 默认返回 0，子类应该重写以从 GAS AttributeSet 或自定义字段返回真实值
	return 0.0f;
}

// ==================== 死亡状态 ====================

EDADeathState ADBACharacterBase::GetDeathState() const
{
	return DeathState;
}

bool ADBACharacterBase::IsDead() const
{
	return DeathState == EDADeathState::Dead || DeathState == EDADeathState::Dying;
}

// ==================== 队伍信息 ====================

int32 ADBACharacterBase::GetTeamID() const
{
	return TeamID;
}

void ADBACharacterBase::SetTeamID(int32 NewTeamID)
{
	if (HasAuthority())
	{
		TeamID = NewTeamID;
	}
	else
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[ADBACharacterBase] 设置队伍 ID 失败：仅在服务端可调用，当前为客户端。请求的 TeamID=%d"), NewTeamID);
	}
}

// ==================== 网络复制 ====================

void ADBACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 死亡状态：所有客户端可见（用于播放死亡动画、隐藏角色等）
	DOREPLIFETIME(ADBACharacterBase, DeathState);

	// 队伍 ID：所有客户端可见（用于阵营识别、伤害判定、UI 显示）
	DOREPLIFETIME(ADBACharacterBase, TeamID);
}
