// Copyright Freebooz Games, Inc. All Rights Reserved.
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