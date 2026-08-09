// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 本子系统是角色选择页到 Dedicated Server 的唯一客户端业务入口。
- Widget 只能向 FrontendFlow 提交“进入游戏”意图；本类负责调用 ApiClient、等待服务器就绪、保管短时 Ticket 并交给 TravelCoordinator。
- AccessToken、RefreshToken 和 GameTicket 不进入 ViewModel、Widget 或 FrontendSessionContext，也不写入日志。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/Core/DBAResultTypes.h"
#include "DBAGameSessionSubsystem.generated.h"

/**
 * 已通过服务端校验的临时连接信息。它不带 UPROPERTY，防止 Blueprint/UI 无意长期持有 GameTicket。
 */
struct FDBAEnterWorldConnection
{
	FString SessionId;
	FString ServerHost;
	int32 ServerPort = 0;
	FString GameTicket;
	FString PlayerId;
	FString CharacterId;
	FString ServerInstanceId;
	FString BuildId;
	int32 TeamId = 0;
	FString Zodiac;
	FString PrimaryElement;
	FString FiveCamp;
	FString FixedSkillGroupId;

	bool IsValid() const
	{
		return !SessionId.IsEmpty() && !ServerHost.IsEmpty() && ServerPort > 0
			&& !GameTicket.IsEmpty() && !PlayerId.IsEmpty() && !CharacterId.IsEmpty()
			&& !ServerInstanceId.IsEmpty() && !BuildId.IsEmpty();
	}
};

/** 进入世界完成时只向 Flow 返回结构化成功/错误，不向 UI 暴露 Ticket。 */
using FDBAEnterGameCompletion = TFunction<void(const FDBAOperationResult&)>;

/**
 * 前台唯一入服业务入口。它复用 UDBAApiClientSubsystem，不直接持有 HTTP 传输对象，
 * 并通过请求代次防止换服、登出或取消之后的旧回调触发 ClientTravel。
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAGameSessionSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedInCurrentEnvironment() const override;
	virtual void OnSubsystemDeinitialize() override;

	/**
	 * 请求进入指定角色所在区服。服务器未就绪时内部以同一业务意图重试，
	 * 成功后立即交给 TravelCoordinator；不会把 AccessToken 作为 DS 准入票据。
	 */
	void EnterGame(const FString& CharacterId, const FString& ServerId, FDBAEnterGameCompletion Completion);

	/** 供换服、登出、Token 失效和 Flow 回退统一调用，取消等待与未完成 HTTP 请求。 */
	void CancelEnterGame();

	bool IsEnterGameInFlight() const { return bEnterGameInFlight; }

protected:
	virtual void CancelAllAsyncOperations() override { CancelEnterGame(); }

private:
	void SendEnterRequest(uint64 RequestGeneration);
	void HandleEnterResponse(uint64 RequestGeneration, const struct FDBAApiResponse& Response);
	void ScheduleRetry(uint64 RequestGeneration);
	void CompleteEnter(uint64 RequestGeneration, const FDBAOperationResult& Result);
	bool ParseEnterResponse(const FString& DomainJson, FString& OutStatus, FDBAEnterWorldConnection& OutConnection) const;

	uint64 EnterRequestGeneration = 0;
	FGuid ActiveApiRequestId;
	FTimerHandle RetryTimerHandle;
	FString PendingCharacterId;
	FString PendingServerId;
	int32 PendingAttempt = 0;
	bool bEnterGameInFlight = false;
	FDBAEnterGameCompletion PendingCompletion;
};
