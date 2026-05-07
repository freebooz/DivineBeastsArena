// Copyright Freebooz Games, Inc. All Rights Reserved.

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
	UPROPERTY()
	FVector PredictedLocation;

	/** 最后服务端校正位置 */
	UPROPERTY()
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
