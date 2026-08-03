// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Data/DBADataAssetBase.h"
#include "UObject/SoftObjectPtr.h"
#include "DBAUIFlowRegistry.generated.h"

class UDBAArenaHUDRootWidgetBase;
class UDBACharacterCreateFlowWidgetBase;
class UDBACharacterSelectFlowWidgetBase;
class UDBAGameSettingsWidgetBase;
class UDBAInteractionPromptWidgetBase;
class UDBAInventoryWidgetBase;
class UDBAInvitePanelWidgetBase;
class UDBALoadingScreenWidgetBase;
class UDBALobbyPlayerHUDWidgetBase;
class UDBALoginFlowWidgetBase;
class UDBAMainLobbyWidgetBase;
class UDBAMatchFoundWidgetBase;
class UDBANewbieTaskTrackerWidgetBase;
class UDBANewbieVillageMainWidgetBase;
class UDBAPartyPanelWidgetBase;
class UDBAPortalConfirmWidgetBase;
class UDBAQueueModeSelectWidgetBase;
class UDBAQueueStatusWidgetBase;
class UDBAReadyCheckWidgetBase;
class UDBASplashVideoWidget;

/**
 * 前端与大厅 UI 类注册表。
 *
 * 该资产是 Widget Blueprint 类引用的唯一配置入口。运行时管理器只负责异步加载、
 * 生命周期与状态切换，不再在 C++ 中保存具体资产路径。
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAUIFlowRegistry : public UDBADataAssetBase
{
	GENERATED_BODY()

public:
	/** 收集需要异步加载的全部 Widget 类路径。 */
	void GetWidgetClassPaths(TArray<FSoftObjectPath>& OutPaths) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Frontend")
	TSoftClassPtr<UDBALoginFlowWidgetBase> LoginWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Frontend")
	TSoftClassPtr<UDBACharacterSelectFlowWidgetBase> CharacterSelectWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Frontend")
	TSoftClassPtr<UDBACharacterCreateFlowWidgetBase> CharacterCreateWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Frontend")
	TSoftClassPtr<UDBASplashVideoWidget> SplashVideoWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAMainLobbyWidgetBase> MainLobbyWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBALobbyPlayerHUDWidgetBase> LobbyPlayerHUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBALoadingScreenWidgetBase> LobbyLoadingWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAGameSettingsWidgetBase> GameSettingsWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAInventoryWidgetBase> InventoryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAPartyPanelWidgetBase> PartyPanelWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAInvitePanelWidgetBase> InvitePanelWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAQueueModeSelectWidgetBase> QueueModeSelectWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAQueueStatusWidgetBase> QueueStatusWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAReadyCheckWidgetBase> ReadyCheckWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAMatchFoundWidgetBase> MatchFoundWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAPortalConfirmWidgetBase> PortalConfirmWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBAInteractionPromptWidgetBase> InteractionPromptWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBANewbieVillageMainWidgetBase> NewbieVillageMainWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Lobby")
	TSoftClassPtr<UDBANewbieTaskTrackerWidgetBase> NewbieTaskTrackerWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Arena")
	TSoftClassPtr<UDBAArenaHUDRootWidgetBase> ArenaHUDWidgetClass;
};
