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
#include "GameCore/Account/DBAAccountTypes.h"
#include "GameCore/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "DBALoginFlowSubsystem.generated.h"

/**
 * 登录流程状态枚举
 */
UENUM(BlueprintType)
enum class EDBALoginFlowState : uint8
{
	Startup,			/** 启动中 */
	TryAutoLogin,		/** 尝试自动登录 */
	LoginScreen,		/** 登录界面 */
	LoadCharacterList,	/** 加载角色列表 */
	CharacterSelect,	/** 角色选择 */
	CharacterCreate,	/** 角色创建 */
	MainLobby,			/** 主大厅 */
	Error				/** 错误状态 */
};

/**
 * 登录流程委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowStateChanged, EDBALoginFlowState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowCharactersLoaded, const TArray<FDBACharacterSummary>&, Characters);

/**
 * 登录流程子系统
 *
 * 管理玩家从启动到进入主大厅的完整登录流程
 *
 * 功能说明：
 * - 处理邮箱/游客/调试等多种登录方式
 * - 管理角色列表加载、选择、创建流程
 * - 协调后端认证和前端会话
 * - 提供登录状态广播和错误处理
 *
 * 状态流转：
 * Startup -> TryAutoLogin -> LoginScreen (或直接到 CharacterSelect)
 * LoginScreen -> SubmitLogin/SubmitGuestLogin -> TryAutoLogin -> LoadCharacterList
 * LoadCharacterList -> CharacterSelect (有角色) 或 CharacterCreate (无角色)
 * CharacterSelect/CharacterCreate -> MainLobby
 *
 * 使用示例：
 * - 获取子系统并监听状态变化: LoginFlowSubsystem->OnFlowStateChanged.AddDynamic(...)
 * - 提交登录: LoginFlowSubsystem->SubmitLogin(Email, Password)
 * - 创建角色: LoginFlowSubsystem->SubmitCharacterCreation(Request)
 */
UCLASS()
class GAMECORE_API UDBALoginFlowSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void StartLoginFlow();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitLogin(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitGuestLogin();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitDebugLogin(const FString& Username = TEXT("dba_dev_01"));

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitCharacterSelection(const FDBACharacterId& CharacterId);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitCharacterCreation(const FDBACharacterCreateRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void EnterCharacterCreate();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void BackToCharacterSelect();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void RefreshCharacterList();

	UFUNCTION(BlueprintPure, Category = "DBA|LoginFlow")
	EDBALoginFlowState GetFlowState() const { return FlowState; }

	UFUNCTION(BlueprintPure, Category = "DBA|LoginFlow")
	const TArray<FDBACharacterSummary>& GetCachedCharacters() const { return CachedCharacters; }

	UFUNCTION(BlueprintPure, Category = "DBA|LoginFlow")
	static bool ShouldEnterCharacterCreate(int32 CharacterCount);

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowStateChanged OnFlowStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowError OnFlowError;

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowCharactersLoaded OnCharactersLoaded;

protected:
	UPROPERTY()
	EDBALoginFlowState FlowState = EDBALoginFlowState::Startup;

	UPROPERTY()
	TArray<FDBACharacterSummary> CachedCharacters;

	UPROPERTY()
	EDBAZodiac CurrentSelectedLobbyZodiac = EDBAZodiac::None;

	void SetFlowState(EDBALoginFlowState NewState);
	void LoadCharactersAfterLogin();
	void FetchPostLoginDataAndEnterLobby();
	void EnterMainLobby();
	void BroadcastErrorAndSetState(const FString& ErrorMessage, EDBALoginFlowState NewState);

	UFUNCTION()
	void HandleDebugLoginAuthResponse(bool bSuccess, const FString& ErrorMessage, const FString& AccessToken, const FString& RefreshToken, const FString& PlayerId);

	UFUNCTION()
	void HandleDebugLoginProfileResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleDebugLoginConfigResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);
};
