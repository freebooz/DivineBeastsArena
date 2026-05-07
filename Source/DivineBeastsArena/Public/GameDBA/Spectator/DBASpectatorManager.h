// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Spectator/DBAObserverTypes.h"
#include "DBASpectatorManager.generated.h"

class ADBAGameModeBase;

/**
 * FDBAObserverPlayerEntry
 * 观战玩家条目 (内部用)
 */
USTRUCT()
struct FDBAObserverPlayerEntry
{
	GENERATED_BODY()

	FDBAObserverPlayerEntry()
		: Character(nullptr)
	{}

	FDBAObserverPlayerEntry(ADBAZodiacCharacterBase* InCharacter)
		: Character(InCharacter)
	{}

	UPROPERTY()
	TWeakObjectPtr<ADBAZodiacCharacterBase> Character;
};

/**
 * FOnObserverViewTargetChangedDelegate
 * 视角切换委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObserverViewTargetChanged, int32, OldIndex, int32, NewIndex);

/**
 * FOnObserverConnectedDelegate
 * 观战者连接委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObserverConnected, const FDBAObserverInfo&, ObserverInfo);

/**
 * FOnObserverDisconnectedDelegate
 * 观战者断开委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObserverDisconnected, const FUniqueNetIdRepl&, ObserverID);

/**
 * FOnMatchPausedDelegate
 * 比赛暂停委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchPaused, bool, bIsPaused);

/**
 * DBASpectatorManager
 * 观战管理器
 * 负责管理观战者连接、视角切换、观战数据同步
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBASpectatorManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDBASpectatorManager();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	/** 连接观战者 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	bool ConnectObserver(APlayerController* ObserverController, EDBAObserverControlLevel ControlLevel);

	/** 断开观战者 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	void DisconnectObserver(APlayerController* ObserverController);

	/** 根据MatchID连接观战者 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	bool ConnectToMatch(APlayerController* ObserverController, FString MatchID, EDBAObserverControlLevel ControlLevel);

	/** 获取观战者列表 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	TArray<FDBAObserverInfo> GetObserverList() const;

	/** 获取观战者信息 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	FDBAObserverInfo GetObserverInfo(APlayerController* ObserverController) const;

	/** 切换视角到下一个玩家 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	void CycleToNextTarget(APlayerController* ObserverController);

	/** 切换视角到上一个玩家 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	void CycleToPreviousTarget(APlayerController* ObserverController);

	/** 直接切换到指定索引玩家 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	bool SetViewTargetByIndex(APlayerController* ObserverController, int32 TargetIndex);

	/** 获取当前视角目标 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	FDBAObserverViewTarget GetCurrentViewTarget(APlayerController* ObserverController) const;

	/** 获取所有可用的视角目标 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	TArray<FDBAObserverViewTarget> GetAllViewTargets() const;

	/** 设置视角模式 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	void SetViewMode(APlayerController* ObserverController, EDBAObserverViewMode ViewMode);

	/** 暂停/恢复比赛 (需要权限) */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	bool TogglePause(APlayerController* RequesterController);

	/** 检查比赛是否已暂停 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	bool IsPaused() const { return bIsPaused; }

	/** 踢出观战者 (需要权限) */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	bool KickObserver(APlayerController* RequesterController, APlayerController* ObserverToKick);

	/** 检查观战者是否有指定权限 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	bool HasControlLevel(APlayerController* ObserverController, EDBAObserverControlLevel RequiredLevel) const;

public:
	/** 视角切换事件 */
	UPROPERTY(BlueprintAssignable, Category = "DBA|Spectator|Event")
	FOnObserverViewTargetChanged OnViewTargetChanged;

	/** 观战者连接事件 */
	UPROPERTY(BlueprintAssignable, Category = "DBA|Spectator|Event")
	FOnObserverConnected OnObserverConnected;

	/** 观战者断开事件 */
	UPROPERTY(BlueprintAssignable, Category = "DBA|Spectator|Event")
	FOnObserverDisconnected OnObserverDisconnected;

	/** 比赛暂停事件 */
	UPROPERTY(BlueprintAssignable, Category = "DBA|Spectator|Event")
	FOnMatchPaused OnMatchPaused;

protected:
	/** 获取观战者数据 */
	FDBAObserverInfo* GetObserverEntry(APlayerController* ObserverController);
	const FDBAObserverInfo* GetObserverEntry(APlayerController* ObserverController) const;

	/** 更新观战者视角数据 */
	void UpdateObserverViewTargets();

	/** 广播视角数据到所有观战者 */
	void BroadcastViewTargetUpdate();

	/** 获取比赛中的所有玩家 */
	TArray<ADBAZodiacCharacterBase*> GetMatchPlayers() const;

	/** 检查是否有暂停权限 */
	bool CanPause(APlayerController* RequesterController) const;

private:
	/** 观战者信息映射 */
	UPROPERTY()
	TMap<TObjectPtr<APlayerController>, FDBAObserverInfo> ObserverMap;

	/** 当前比赛中的玩家列表 */
	UPROPERTY()
	TArray<FDBAObserverPlayerEntry> MatchPlayers;

	/** 是否已暂停 */
	UPROPERTY()
	bool bIsPaused;

	/** 当前观看的MatchID */
	UPROPERTY()
	FString CurrentMatchID;
};
