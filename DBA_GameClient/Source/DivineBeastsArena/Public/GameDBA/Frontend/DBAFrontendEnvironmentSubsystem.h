// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：前端登录地图环境清理子系统，移除三维光照/大气/云等场景组件。
- 阅读重点：仅在 FrontendMap 客户端生效，登录 UI 由 UMG 背景承担表现。
- 修改提示：不要在此处承载登录 UI 逻辑；仅负责前端关卡环境降级。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAWorldSubsystemBase.h"
#include "DBAFrontendEnvironmentSubsystem.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAFrontendEnvironmentSubsystem : public UDBAWorldSubsystemBase
{
	GENERATED_BODY()

public:
	/** 清理前端登录地图中的三维环境组件（可在世界切换后重复调用） */
	UFUNCTION(BlueprintCallable, Category = "DBA|Frontend")
	void ApplyFrontendUiOnlyEnvironment();

	/** 恢复角色选择/创建界面所需的世界 3D 展示渲染开关 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Frontend")
	void EnableCharacterPresentationRendering();

protected:
	virtual bool IsSupportedInCurrentEnvironment() const override;
	virtual void OnWorldBeginPlayInternal() override;
	virtual void OnSubsystemDeinitialize() override;

private:
	bool IsFrontendWorld() const;
	void ScheduleEnvironmentCleanupRetries();
	void ClearEnvironmentCleanupTimers();
	void HandleFollowUpEnvironmentCleanup();

	FTimerHandle InitialCleanupTimerHandle;
	FTimerHandle FollowUpCleanupTimerHandle;
	int32 EnvironmentCleanupRetryCount = 0;
};
