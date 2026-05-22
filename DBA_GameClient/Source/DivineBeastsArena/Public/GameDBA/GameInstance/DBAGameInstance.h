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
#include "Engine/GameInstance.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "DBAGameInstance.generated.h"

/**
 * DBAGameInstance
 *
 * 游戏实例类
 * 生命周期：整个游戏进程
 * 跨关卡持久化，管理全局子系统
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDBAGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;

	/**
	 * 开始登录流程（由启动视频结束后调用）
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|GameInstance")
	void StartLoginFlow();

private:
	UFUNCTION()
	void HandleAutoLobbyFlowStateChanged(EDBALoginFlowState NewState);

	void TryStartAutoLobbyFlow();
	void ContinueAutoLobbyFlow(EDBALoginFlowState FlowState);
	void RunAutoPartyStep();

	/** 是否已启动登录流程 */
	UPROPERTY(Transient)
	bool bLoginFlowStarted = false;

	UPROPERTY(Transient)
	bool bPendingStartLoginFlowOnFrontend = false;

	UPROPERTY(Transient)
	bool bAutoLobbyFlowStarted = false;

	UPROPERTY(Transient)
	bool bAutoLobbyPartyStepDone = false;
};
