// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 本类独占前台到游戏世界的 ClientTravel 调用，禁止 Widget、ViewModel、Flow 拼接服务器地址或直接 Travel。
- 进入旅行前由本类冻结前台交互、释放角色预览资源；失败时只返回结构化错误，由 Flow 恢复角色选择页。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/Core/DBAResultTypes.h"
#include "DBAFrontendTravelCoordinator.generated.h"

struct FDBAEnterWorldConnection;
class UWorld;

using FDBAFrontendTravelCompletion = TFunction<void(const FDBAOperationResult&)>;

UCLASS()
class DIVINEBEASTSARENA_API UDBAFrontendTravelCoordinator : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedInCurrentEnvironment() const override;
	virtual void OnSubsystemDeinitialize() override;

	/** 只接受已由 GameSessionSubsystem 验证的连接对象；此处不接收 Widget 传来的任意地址。 */
	void TravelToGameWorld(const FDBAEnterWorldConnection& Connection, FDBAFrontendTravelCompletion Completion);
	void CancelPendingTravel();
	bool IsTravelInFlight() const { return bTravelInFlight; }

protected:
	virtual void CancelAllAsyncOperations() override { CancelPendingTravel(); }

private:
	bool BuildTravelUrl(const FDBAEnterWorldConnection& Connection, FString& OutTravelUrl) const;
	void EndLoading();
	/** 引擎确认新地图加载后回收前台 Loading token，避免 GameInstanceSubsystem 跨地图遗留阻塞层。 */
	void HandlePostLoadMap(UWorld* LoadedWorld);

	bool bTravelInFlight = false;
	FName LoadingRequestToken;
	FDelegateHandle PostLoadMapHandle;
};
