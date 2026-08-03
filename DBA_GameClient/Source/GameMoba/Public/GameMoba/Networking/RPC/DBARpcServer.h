// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 服务端RPC接口 - 客户端调用服务端的RPC方法
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBARpcInterface.h"
#include "DBARpcServer.generated.h"

class AActor;

/**
 * UDBARpcServer
 * 服务端RPC接口 - 客户端调用服务端
 */
UINTERFACE(BlueprintType)
class GAMEMOBA_API UDBARpcServer : public UDBARpcInterface
{
    GENERATED_BODY()
};

/**
 * IDBARpcServer
 * 服务端RPC接口实现 - 客户端调用服务端
 */
class GAMEMOBA_API IDBARpcServer : public IDBARpcInterface
{
    GENERATED_BODY()

public:
    // 尝试激活技能
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerTryActivateAbility(const FDBAAbilityRpcParams& Params) = 0;

    // 取消技能
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerCancelAbility(FGameplayAbilitySpecHandle Handle) = 0;

    // 锁定目标
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerLockTarget(AActor* TargetActor) = 0;

    // 移动到位置 (不可靠 - 高频调用)
    UFUNCTION(Server, Unreliable, WithValidation)
    virtual void ServerMoveTo(FVector_NetQuantize10 Location) = 0;

    // 请求攻击
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerRequestAttack() = 0;

    // 终极技能
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerUltimateAbility(const FDBAAbilityRpcParams& Params) = 0;
};
