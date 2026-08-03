// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 角色统一基础类 - 所有角色与怪物的公共父类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameMoba/Targeting/DBACombatTypes.h"
#include "DBACharacterBase.generated.h"

class UAbilitySystemComponent;

/**
 * ADBACharacterBase
 * 角色与怪物的统一抽象基础类
 *
 * 职责：
 *   - 提供 IAbilitySystemInterface 接口的统一接入点（子类通过重写 GetAbilitySystemComponent 决定 ASC 来源）
 *   - 持有所有角色/怪物共有的复制字段：死亡状态、队伍 ID
 *   - 提供生命值、死亡判断、队伍信息等通用访问接口（默认实现返回安全值，子类按需重写）
 *   - 不强制持有 ASC，子类自行决定 ASC 持有位置（玩家角色放 PlayerState，AI 角色放自身组件）
 *
 * 继承体系：
 *   ACharacter + IAbilitySystemInterface
 *   └── ADBACharacterBase                    （本类，抽象）
 *       ├── ADBAZodiacCharacterBase          （生肖角色基类，ASC 在 PlayerState）
 *       │   └── 生肖角色类通过 UDBAZodiacCharacterRegistry 数据资产配置，不再使用子类硬编码
 *       └── ADBAMonsterBase                  （怪物基类，ASC 在自身组件）
 *           └── ADBALobbyTrainingMonster
 *
 * 设计依据：
 *   - 项目策略《全局 C++ 逻辑实现策略》：所有逻辑相关实现使用 C++
 *   - 项目策略《DBA.DataAsset.NoHardcoding》：角色配置通过子类持有的 DataAsset 软引用驱动
 *   - 项目策略《DBA.UI.EventAsync》：异步接口与事件驱动
 *   - 项目策略《DBA.Log.ChineseOutput》：所有日志使用中文输出
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBACharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADBACharacterBase();

	// ==================== IAbilitySystemInterface 实现 ====================

	/**
	 * 获取能力系统组件
	 * 默认返回 nullptr，子类必须重写以提供真实的 ASC：
	 *   - 玩家角色：从 PlayerState 获取（标准做法）
	 *   - AI 角色：从自身 CreateDefaultSubobject 创建的组件获取
	 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// ==================== 角色通用属性访问 ====================

	/**
	 * 获取当前生命值
	 * 默认返回 0，子类应该重写以从 GAS AttributeSet 或自定义字段返回真实值
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	virtual float GetCurrentHealth() const;

	/**
	 * 获取最大生命值
	 * 默认返回 0，子类应该重写以从 GAS AttributeSet 或自定义字段返回真实值
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	virtual float GetMaxHealth() const;

	// ==================== 死亡状态 ====================

	/** 获取死亡状态 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Death")
	virtual EDADeathState GetDeathState() const;

	/** 是否已死亡（包含 Dying 和 Dead 状态） */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Death")
	virtual bool IsDead() const;

	// ==================== 队伍信息 ====================

	/** 获取队伍 ID */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Team")
	virtual int32 GetTeamID() const;

	/** 设置队伍 ID（仅服务端调用） */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Team")
	virtual void SetTeamID(int32 NewTeamID);

protected:
	// ==================== 网络复制 ====================

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==================== 公共复制字段 ====================

	/** 死亡状态（复制） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Death")
	EDADeathState DeathState = EDADeathState::Alive;

	/** 队伍 ID（复制） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Team")
	int32 TeamID = 0;
};
