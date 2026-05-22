// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 怪物AI控制器

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"
#include "DBAMonsterAIController.generated.h"

class UDBAMonsterAIComponent;

/**
 * ADBAMonsterAIController
 * 怪物AI控制器
 * 负责管理AI行为树和黑板数据
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API ADBAMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	ADBAMonsterAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:
	/** 行为树资源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	UBehaviorTree* BehaviorTree;

	/** 黑板数据资源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	UBlackboardData* BlackboardAsset;

public:
	/** 获取AI组件 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	UDBAMonsterAIComponent* GetAIComponent() const { return AIComponent; }

protected:
	/** AI组件弱引用 */
	UPROPERTY(Transient)
	TObjectPtr<UDBAMonsterAIComponent> AIComponent;

private:
	/** 初始化AI组件 */
	void InitializeAIComponent();
};