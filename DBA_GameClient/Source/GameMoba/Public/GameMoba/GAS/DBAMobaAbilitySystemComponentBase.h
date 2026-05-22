// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// GameMoba - 通用MOBA AbilitySystemComponent基类

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DBAMobaAbilitySystemComponentBase.generated.h"

/**
 * UDBAMobaAbilitySystemComponentBase
 * MOBA游戏通用AbilitySystemComponent基类
 * 提供GAS通用接口
 */
UCLASS(ClassGroup=(GameMoba), meta=(BlueprintSpawnableComponent))
class GAMEMOBA_API UDBAMobaAbilitySystemComponentBase : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UDBAMobaAbilitySystemComponentBase(const FObjectInitializer& ObjectInitializer);

protected:
	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

public:
	/** 初始化技能系统 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	virtual void InitializeAbilities(AActor* InOwnerActor, AActor* InAvatarActor);

	/** 清理所有已授予的技能 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	void RemoveAllGrantedAbilities();

protected:
	/** 技能初始化完成 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Ability|Base")
	bool bAbilitiesInitialized = false;

	/** 已授予的技能Handle列表 */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
};