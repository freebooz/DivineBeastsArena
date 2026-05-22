// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMoba/RPC/DBARpcInterface.h"
#include "DBAClientPredictionComponent.generated.h"

/**
 * DBAClientPredictionComponent
 *
 * 客户端预测组件
 * 管理客户端输入预测和服务器校正
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "DBA Client Prediction"))
class DIVINEBEASTSARENA_API UDBAClientPredictionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAClientPredictionComponent();

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

public:
	/** 尝试激活技能 (带预测) */
	UFUNCTION(BlueprintCallable, Category = "DBA|Prediction")
	void TryPredictAbility(FName SkillId, AActor* Target, FVector TargetLocation);

	/** 尝试移动 (带预测) */
	UFUNCTION(BlueprintCallable, Category = "DBA|Prediction")
	void TryPredictMove(FVector TargetLocation);

	/** 应用服务端校正 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Prediction")
	void ApplyServerCorrection(FVector ServerLocation, float ServerTime);

	/** 获取当前预测误差 */
	UFUNCTION(BlueprintPure, Category = "DBA|Prediction")
	float GetPredictionError() const { return PredictionError; }

protected:
	/** 处理技能激活返回 */
	void OnAbilityActivated(FGameplayAbilitySpecHandle Handle, bool bSuccess);

	/** 处理移动校正返回 */
	void OnMoveCorrected(FVector CorrectedLocation);

protected:
	/** 当前预测位置 */
	UPROPERTY(Transient)
	FVector PredictedLocation;

	/** 最后服务端校正位置 */
	UPROPERTY(Transient)
	FVector LastCorrectedLocation;

	/** 预测误差 (用于调试) */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Prediction")
	float PredictionError = 0.0f;

	/** 预测有效期 (秒) */
	UPROPERTY(EditDefaultsOnly, Category = "DBA|Prediction")
	float PredictionTimeout = 0.5f;

	/** 校正平滑时间 (秒) */
	UPROPERTY(EditDefaultsOnly, Category = "DBA|Prediction")
	float CorrectionSmoothingTime = 0.1f;

private:
};
