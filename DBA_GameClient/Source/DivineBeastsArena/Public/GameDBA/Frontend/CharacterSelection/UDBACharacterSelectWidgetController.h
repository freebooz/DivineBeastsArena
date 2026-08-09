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
#include "GameCore/Networking/Account/DBAAccountTypes.h"
#include "GameDBA/Core/DBAResultTypes.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"
#include "GameMoba/UI/DBAMobaHUDWidgetControllerBase.h"
#include "UDBACharacterSelectWidgetController.generated.h"

class UDBAFrontendFlowSubsystem;
class UDBACharacterRosterSubsystem;
class UDBACharacterPreviewSubsystem;
class UDBACharacterSelectViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBACharactersChanged, const TArray<FDBACharacterSummary>&, Characters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBACharacterDeleteConfirmationRequested, const FDBACharacterSummary&, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBACharacterSelectUIError, const FDBAApiError&, Error);

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterSelectWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBACharacterSelectWidgetController(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void BindLoginFlow();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void SelectCharacter(const FDBACharacterId& CharacterId);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void EnterGame();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void CreateCharacter();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void Refresh();

	/** 显示确认状态；由 WBP_DBA_ModalDialog 绑定 ConfirmDelete/CancelDelete。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void RequestDeleteSelectedCharacter();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void ConfirmDelete();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void CancelDelete();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void BackToServerSelect();

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	UDBACharacterSelectViewModel* GetViewModel() const { return ViewModel; }

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterSelect")
	FDBACharactersChanged OnCharactersChanged;

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterSelect")
	FDBACharacterDeleteConfirmationRequested OnDeleteConfirmationRequested;

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterSelect")
	FDBACharacterSelectUIError OnCharacterSelectError;

protected:
	UFUNCTION()
	void HandleCharactersLoaded(const TArray<FDBACharacterSummary>& Characters);
	void HandleRosterChanged(const TArray<FDBACharacterSummary>& Characters);
	void HandlePreviewResolved(EDBAZodiac Zodiac, bool bSuccess);
	void HandleFlowStateChanged(EDBAFrontendState PreviousState, EDBAFrontendState NewState);
	void ApplySelection(const FDBACharacterId& CharacterId);
	void PublishError(const FDBAOperationResult& Result);
	void UnbindServices();

	UDBAFrontendFlowSubsystem* GetLoginFlow() const;
	UDBACharacterRosterSubsystem* GetRoster() const;
	UDBACharacterPreviewSubsystem* GetPreviewSubsystem() const;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterSelectViewModel> ViewModel;

	TWeakObjectPtr<UDBACharacterRosterSubsystem> Roster;
	TWeakObjectPtr<UDBACharacterPreviewSubsystem> PreviewSubsystem;
	FDelegateHandle RosterChangedHandle;
	FDelegateHandle PreviewResolvedHandle;
};
