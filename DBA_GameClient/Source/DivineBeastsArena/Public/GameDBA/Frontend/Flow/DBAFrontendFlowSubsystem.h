// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameCore/Networking/Account/DBAAccountTypes.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/Core/DBAResultTypes.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"
#include "GameDBA/Frontend/Flow/DBAFrontendStateMachine.h"
#include "DBAFrontendFlowSubsystem.generated.h"

/**
 * 旧登录流程的只读视图状态。
 *
 * 该类型仅为尚未迁移的 UMG/Blueprint 与大厅兼容代码保留；新业务必须读取
 * EDBAFrontendState，并订阅 OnFrontendStateChanged，禁止继续扩展本枚举。
 */
UENUM(BlueprintType, meta = (Deprecated = "5.8", DeprecationMessage = "请使用 EDBAFrontendState；EDBALoginFlowState 仅保留为旧界面的只读投影。"))
enum class EDBALoginFlowState : uint8
{
	Booting,
	AwaitingLogin,
	Authenticating,
	ServerSelecting,
	LoadingCharacters,
	CharacterSelecting,
	CharacterCreating,
	AllocatingVillage,
	WaitingVillageServer,
	ConnectingVillage,
	InitializingVillage,
	InVillage,
	PartyPreparing,
	Matchmaking,
	MatchReadyCheck,
	AllocatingArena,
	ConnectingArena,
	InitializingArena,
	InArena,
	ShowingMatchResult,
	AllocatingReturnVillage,
	ReturningVillage,
	RecoverableError,
	FatalError
};

/**
 * 登录流程委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowStateChanged, EDBALoginFlowState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowApiError, const FDBAApiError&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowCharactersLoaded, const TArray<FDBACharacterSummary>&, Characters);
/** Confirm 页面用此事件结束 Loading；失败时 Draft 保持不变，成功时摘要已进入 Roster 缓存。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDBAOnCharacterCreateCompleted, const FDBAOperationResult&, Result, const FDBACharacterSummary&, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDBAOnFrontendStateChanged, EDBAFrontendState, PreviousState, EDBAFrontendState, NewState);

/**
 * 登录流程子系统
 *
 * 管理玩家从启动到进入主大厅的完整登录流程
 *
 * 功能说明：
 * - 处理邮箱和游客登录
 * - 管理角色列表加载、选择、创建流程
 * - 协调后端认证和前端会话
 * - 提供登录状态广播和错误处理
 *
 * 状态流转：
 * Startup -> LoginScreen
 * LoginScreen -> SubmitLogin/SubmitGuestLogin -> TryAutoLogin -> LoadCharacterList
 * LoadCharacterList -> CharacterSelect (有角色) 或 CharacterCreate (无角色)
 * CharacterSelecting/CharacterCreating -> InVillage
 *
 * 使用示例：
 * - 获取子系统并监听状态变化: LoginFlowSubsystem->OnFlowStateChanged.AddDynamic(...)
 * - 提交登录: LoginFlowSubsystem->SubmitLogin(Email, Password)
 * - 创建角色: LoginFlowSubsystem->SubmitCharacterCreation(Request)
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAFrontendFlowSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void StartLoginFlow();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitLogin(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitGuestLogin();

	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	void BeginRegistration();

	/** 注册成功后与账号登录走同一条选服链路；密码仅在本次异步请求中使用。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	void SubmitRegistration(const FString& Account, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	void CancelRegistration();

	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	void BeginServerSelection();

	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	void SelectServer(const FString& ServerId);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitCharacterSelection(const FDBACharacterId& CharacterId);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitCharacterCreation(const FDBACharacterCreateRequest& Request);

	/**
	 * 仅取消仍在本地等待的创建 HTTP 请求并保留 Draft/幂等键，以便安全重试；
	 * 不把未创建角色当作服务器实体，也不进行页面跳转。
	 */
	void CancelCharacterCreationSubmission();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void EnterCharacterCreate();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void BackToCharacterSelect();

	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	void CancelCharacterCreation();

	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	bool SelectCharacterCreateZodiac(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	bool SelectCharacterCreateElement(EDBAElement Element);

	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	bool SelectCharacterCreateFiveCamp(EDBAFiveCamp FiveCamp);

	/** 推进账号角色创建 Draft 的当前步骤；赛前选择不得调用该入口。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	bool AdvanceCharacterCreateDraft();

	/** 返回账号角色创建的上一步骤；第一步返回角色选择并清理草稿。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	void BackCharacterCreateStep();

	UFUNCTION(BlueprintCallable, Category = "DBA|FrontendFlow")
	void RequestLogout();

	/** 鉴权刷新失败或 401 时由 API 层调用，统一回退至 Login。 */
	void HandleTokenExpired();

	/** 服务目录/进服服务不可用时由 API 层调用，统一回退至可操作页面。 */
	void HandleServerUnavailable();

	/** 网络中断时保留当前可恢复 Screen，并通过统一状态事件交给 NetworkStatus/Retry UI。 */
	void HandleNetworkLost();

	/**
	 * 后端维护恢复策略。目录范围维护回选服；全局范围维护回启动页，
	 * 两种情况均先清理角色、预览与创建草稿上下文。
	 */
	void HandleBackendMaintenance(bool bAffectsServerDirectory);

	/** Android 等平台从后台恢复后调用；不会创建第二个 UI Root。 */
	void HandleApplicationResumed();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void RefreshCharacterList();

	UFUNCTION(BlueprintPure, Category = "DBA|LoginFlow", meta = (DeprecatedFunction, DeprecationMessage = "请使用 GetFrontendState。"))
	EDBALoginFlowState GetFlowState() const { return FlowState; }

	/** 唯一前台状态机的权威状态。 */
	UFUNCTION(BlueprintPure, Category = "DBA|FrontendFlow")
	EDBAFrontendState GetFrontendState() const { return FrontendSessionContext.ClientSessionState; }

	UFUNCTION(BlueprintPure, Category = "DBA|FrontendFlow")
	const FDBAFrontendSessionContext& GetFrontendSessionContext() const { return FrontendSessionContext; }

	UFUNCTION(BlueprintPure, Category = "DBA|LoginFlow")
	const TArray<FDBACharacterSummary>& GetCachedCharacters() const { return CachedCharacters; }

	/** 仅供 CharacterRosterSubsystem 写入兼容投影；UI 仍只读取领域摘要。 */
	void ApplyCharacterRosterSnapshot(const TArray<FDBACharacterSummary>& Characters);
	void SetSelectedCharacterFromRoster(const FDBACharacterSummary& Character);
	void ClearSelectedCharacterFromRoster();

	UFUNCTION(BlueprintPure, Category = "DBA|LoginFlow")
	static bool ShouldEnterCharacterCreate(int32 CharacterCount);

	/** 本地玩家的 PlayerState 已在大厅服务器完成复制后调用。 */
	void ConfirmVillageConnectionReady();

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow", meta = (DeprecatedProperty, DeprecationMessage = "请订阅 OnFrontendStateChanged。"))
	FDBAOnLoginFlowStateChanged OnFlowStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowError OnFlowError;

	/** 新 UI 仅订阅该结构化错误事件；OnFlowError 仅为已存在蓝图的兼容出口。 */
	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowApiError OnFlowApiError;

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowCharactersLoaded OnCharactersLoaded;

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnCharacterCreateCompleted OnCharacterCreateCompleted;

	/** 所有新 Screen、WidgetController 与 ViewModel 订阅此事件驱动 UI 路由。 */
	UPROPERTY(BlueprintAssignable, Category = "DBA|FrontendFlow")
	FDBAOnFrontendStateChanged OnFrontendStateChanged;

protected:
	UPROPERTY()
	EDBALoginFlowState FlowState = EDBALoginFlowState::Booting;

	UPROPERTY()
	FDBAFrontendSessionContext FrontendSessionContext;

	UPROPERTY()
	TArray<FDBACharacterSummary> CachedCharacters;

	/**
	 * 异步流程代次。每次发起新的登录、列表、创建或选择请求时递增，
	 * 旧请求回调不得再覆盖当前界面状态。
	 */
	uint64 FlowRequestGeneration = 0;
	/** 旧 Village 两段式链路的兼容状态；新角色选择已不再调用，待大厅连接链路迁移后删除。 */
	uint64 PendingVillageRequestGeneration = 0;
	FString PendingVillageSessionId;
	int32 VillageConnectionAttempt = 0;
	FTimerHandle VillageConnectionRetryTimerHandle;
	bool bAuthenticationRequestInFlight = false;
	/** Android 恢复期间的会话刷新门闩，防止前后台抖动产生并行 Refresh。 */
	bool bResumeSessionRefreshInFlight = false;
	bool bApplicationWasSuspended = false;
	/** 同一 Draft 的创建请求单飞门闩；用于阻止双击 Create 生成两个服务端请求。 */
	bool bCharacterCreateRequestInFlight = false;
	/** 仅用于 HTTP 幂等头，不写日志、不暴露给 Widget；取消后重试复用同一键。 */
	FString PendingCharacterCreateIdempotencyKey;
	FDelegateHandle AuthenticationRefreshFailedHandle;
	FDelegateHandle ApplicationWillEnterBackgroundHandle;
	FDelegateHandle ApplicationHasEnteredForegroundHandle;

	/** 统一回收角色域状态；换服不清账号，登出/全局维护可一并清区服。 */
	void ClearFrontendCharacterContext(bool bClearServerId);
	void RefreshServerDirectory();
	void HandleAuthenticationRefreshFailed();
	void HandleApplicationEnteredBackground();
	void SetFlowState(EDBALoginFlowState NewState);
	bool TryTransitionTo(EDBAFrontendState NewState);
	void ForceRecoverableErrorAndFallback(EDBAFrontendState FallbackState, const FString& ErrorMessage);
	void UpdateLegacyFlowState(EDBAFrontendState State);
	EDBAFrontendState ResolveFrontendStateForLegacyState(EDBALoginFlowState LegacyState) const;
	void StartAutoLogin();
	void HandleLoginSucceeded(const FDBALoginResponse& Response);
	uint64 BeginFlowRequest();
	void InvalidatePendingFlowRequests();
	bool IsFlowRequestCurrent(uint64 RequestGeneration) const;
	void LoadCharactersAfterLogin();
	bool SynchronizeBackendAuthentication();
	/** @deprecated 新角色选择必须走 UDBAGameSessionSubsystem。此入口仅保留给既有大厅兼容链路。 */
	void RequestVillageAllocation();
	void RequestVillageConnection();
	void ScheduleVillageConnectionRetry();
	EDBALoginFlowState ResolveCharacterFlowState() const;
	void BroadcastErrorAndSetState(const FString& ErrorMessage, EDBALoginFlowState NewState);

	/** @deprecated 旧 Village 两段式回调；新的 /api/v1/game/enter 不经过这里。 */
	UFUNCTION()
	void HandleVillageAllocationResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleVillageConnectionResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

};
