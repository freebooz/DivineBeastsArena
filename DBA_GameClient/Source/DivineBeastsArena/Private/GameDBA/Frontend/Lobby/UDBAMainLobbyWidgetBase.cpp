// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/Lobby/UDBAMainLobbyWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/Controllers/DBAGameUIManager.h"
#include "GameDBA/Frontend/Lobby/UDBAMainLobbyWidgetController.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAPartyPanelWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAQueueModeSelectWidgetBase.h"

namespace
{
	UWidget* FindLobbyWidgetByNames(UWidgetTree* WidgetTree, const TArray<FName>& Names)
	{
		if (!WidgetTree)
		{
			return nullptr;
		}

		for (const FName Name : Names)
		{
			if (UWidget* Widget = WidgetTree->FindWidget(Name))
			{
				return Widget;
			}
		}
		return nullptr;
	}

	FText ToBackendStateText(EDBALobbyBackendState State)
	{
		switch (State)
		{
		case EDBALobbyBackendState::Idle:
			return NSLOCTEXT("DBAMainLobby", "BackendStateIdle", "空闲");
		case EDBALobbyBackendState::Loading:
			return NSLOCTEXT("DBAMainLobby", "BackendStateLoading", "加载中");
		case EDBALobbyBackendState::InRoom:
			return NSLOCTEXT("DBAMainLobby", "BackendStateInRoom", "房间中");
		case EDBALobbyBackendState::Ready:
			return NSLOCTEXT("DBAMainLobby", "BackendStateReady", "已准备");
		case EDBALobbyBackendState::Matching:
			return NSLOCTEXT("DBAMainLobby", "BackendStateMatching", "匹配中");
		case EDBALobbyBackendState::MatchFound:
			return NSLOCTEXT("DBAMainLobby", "BackendStateMatchFound", "匹配成功");
		case EDBALobbyBackendState::Connecting:
			return NSLOCTEXT("DBAMainLobby", "BackendStateConnecting", "连接战斗服务器");
		case EDBALobbyBackendState::Error:
			return NSLOCTEXT("DBAMainLobby", "BackendStateError", "错误");
		default:
			return NSLOCTEXT("DBAMainLobby", "BackendStateUnknown", "未知");
		}
	}

	UDBAGameUIManager* ResolveLobbyUIManager(const UUserWidget* Widget)
	{
		const UWorld* World = Widget ? Widget->GetWorld() : nullptr;
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UDBAGameUIManager>() : nullptr;
	}
}

UDBAMainLobbyWidgetBase::UDBAMainLobbyWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentFiveCampTheme(EDBAFiveCamp::Center)
{
}

void UDBAMainLobbyWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	TryBindWidgetController();
}

void UDBAMainLobbyWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	BindBackendUiControls();
	BindControllerDelegates();

	RefreshPartyInfo();
	if (WidgetController)
	{
		HandlePlayerSummaryUpdated(WidgetController->GetPlayerSummary());
		HandleRecentMatchSummaryUpdated(WidgetController->GetRecentMatchSummary());
		HandleBackendStateChanged(WidgetController->GetBackendState());
		WidgetController->InitializeBackendLobby();
	}
}

void UDBAMainLobbyWidgetBase::NativeDestruct()
{
	UnbindControllerDelegates();

	if (CreateRoomButton)
	{
		CreateRoomButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleCreateRoomClicked);
	}
	if (RefreshRoomsButton)
	{
		RefreshRoomsButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleRefreshRoomsClicked);
	}
	if (RefreshMatchHistoryButton)
	{
		RefreshMatchHistoryButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleRefreshMatchHistoryClicked);
	}
	if (JoinRoomButton)
	{
		JoinRoomButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleJoinRoomClicked);
	}
	if (ReadyButton)
	{
		ReadyButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleReadyClicked);
	}
	if (StartGameButton)
	{
		StartGameButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleStartGameClicked);
	}
	if (StartMatchButton)
	{
		StartMatchButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleStartMatchClicked);
	}
	if (CancelMatchButton)
	{
		CancelMatchButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleCancelMatchClicked);
	}

	Super::NativeDestruct();
}

void UDBAMainLobbyWidgetBase::TryBindWidgetController()
{
	if (WidgetController)
	{
		return;
	}

	WidgetController = NewObject<UDBAMainLobbyWidgetController>(this, UDBAMainLobbyWidgetController::StaticClass());
	if (WidgetController)
	{
		WidgetController->InitializeController();
	}
}

void UDBAMainLobbyWidgetBase::SetWidgetController(UDBAMainLobbyWidgetController* InController)
{
	if (WidgetController == InController)
	{
		return;
	}

	UnbindControllerDelegates();
	WidgetController = InController;
	BindControllerDelegates();
}

void UDBAMainLobbyWidgetBase::RefreshPartyInfo()
{
	if (WidgetController)
	{
		WidgetController->RequestPartyInfo();
	}

	BP_OnPartyInfoRefreshed();
}

void UDBAMainLobbyWidgetBase::SwitchFiveCampTheme(EDBAFiveCamp FiveCamp)
{
	CurrentFiveCampTheme = FiveCamp;
	BP_OnFiveCampThemeSwitched(FiveCamp);
}

void UDBAMainLobbyWidgetBase::NavigateToNewbieVillage()
{
	if (WidgetController)
	{
		WidgetController->RequestNavigateToNewbieVillage();
	}
	if (UDBAGameUIManager* UIManager = ResolveLobbyUIManager(this))
	{
		UIManager->ShowNewbieVillageMain();
		UIManager->ShowNewbieTaskTracker();
	}
}

void UDBAMainLobbyWidgetBase::NavigateToPractice()
{
	if (WidgetController)
	{
		WidgetController->RequestNavigateToPractice();
	}
}

void UDBAMainLobbyWidgetBase::OpenQueueModeSelect()
{
	if (QueueModeSelect)
	{
		QueueModeSelect->SetVisibility(ESlateVisibility::Visible);
		return;
	}

	if (UDBAGameUIManager* UIManager = ResolveLobbyUIManager(this))
	{
		UIManager->ShowQueueModeSelect();
	}
}

void UDBAMainLobbyWidgetBase::OpenFriendList()
{
	if (UDBAGameUIManager* UIManager = ResolveLobbyUIManager(this))
	{
		UIManager->ShowInvitePanel();
	}
}

void UDBAMainLobbyWidgetBase::OpenSettings()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->ShowGameSettings();
		}
	}
}

void UDBAMainLobbyWidgetBase::ExitGame()
{
}

void UDBAMainLobbyWidgetBase::BackendRefreshPlayerData()
{
	if (WidgetController)
	{
		WidgetController->RefreshPlayerData();
	}
}

void UDBAMainLobbyWidgetBase::BackendRefreshRoomList()
{
	if (WidgetController)
	{
		WidgetController->RefreshRoomList();
	}
}

void UDBAMainLobbyWidgetBase::BackendRefreshMatchHistory()
{
	if (WidgetController)
	{
		WidgetController->RefreshMatchHistory();
	}
}

void UDBAMainLobbyWidgetBase::BackendCreateRoom()
{
	if (WidgetController)
	{
		WidgetController->CreateRoom();
	}
}

void UDBAMainLobbyWidgetBase::BackendJoinRoom(const FString& RoomId)
{
	if (WidgetController)
	{
		WidgetController->JoinRoom(RoomId);
	}
}

void UDBAMainLobbyWidgetBase::BackendSetReady(bool bReady)
{
	if (WidgetController)
	{
		WidgetController->SetReady(bReady);
	}
}

void UDBAMainLobbyWidgetBase::BackendStartRoom()
{
	if (WidgetController)
	{
		WidgetController->StartRoom();
	}
}

void UDBAMainLobbyWidgetBase::BackendStartMatchmaking(const FString& Mode, const FString& RegionCode)
{
	if (WidgetController)
	{
		WidgetController->StartMatchmaking(Mode, RegionCode);
	}
}

void UDBAMainLobbyWidgetBase::BackendCancelMatchmaking()
{
	if (WidgetController)
	{
		WidgetController->CancelMatchmaking();
	}
}

void UDBAMainLobbyWidgetBase::BackendJoinRoomFromInput()
{
	const FString RoomId = JoinRoomIdInput ? JoinRoomIdInput->GetText().ToString().TrimStartAndEnd() : FString();
	BackendJoinRoom(RoomId);
}

void UDBAMainLobbyWidgetBase::HandleBackendStateChanged(EDBALobbyBackendState NewState)
{
	UpdateBackendStateText(NewState);
	UpdateBackendButtonsState(NewState);

	if (UDBAGameUIManager* UIManager = ResolveLobbyUIManager(this))
	{
		switch (NewState)
		{
		case EDBALobbyBackendState::Matching:
			UIManager->HideMatchFound();
			UIManager->ShowQueueStatus(
				NSLOCTEXT("DBAMainLobby", "QueueModeDefault", "快速匹配"),
				NSLOCTEXT("DBAMainLobby", "QueueMapDefault", "五行竞技场"),
				NSLOCTEXT("DBAMainLobby", "QueueEstimateDefault", "约 2-5 分钟"));
			break;
		case EDBALobbyBackendState::MatchFound:
			UIManager->HideQueueStatus();
			UIManager->ShowMatchFound(
				NSLOCTEXT("DBAMainLobby", "MatchFoundModeDefault", "快速匹配"),
				NSLOCTEXT("DBAMainLobby", "MatchFoundMapDefault", "五行竞技场"));
			break;
		case EDBALobbyBackendState::Connecting:
			UIManager->HideQueueStatus();
			UIManager->HideMatchFound();
			UIManager->HideReadyCheck();
			UIManager->ShowLobbyLoadingScreen();
			break;
		case EDBALobbyBackendState::Idle:
		case EDBALobbyBackendState::InRoom:
		case EDBALobbyBackendState::Ready:
		case EDBALobbyBackendState::Error:
			UIManager->HideQueueStatus();
			UIManager->HideMatchFound();
			UIManager->HideReadyCheck();
			break;
		default:
			break;
		}
	}
}

void UDBAMainLobbyWidgetBase::HandleBackendError(const FString& ErrorMessage)
{
	if (BackendErrorText)
	{
		BackendErrorText->SetText(FText::FromString(ErrorMessage));
		BackendErrorText->SetVisibility(ErrorMessage.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void UDBAMainLobbyWidgetBase::HandlePlayerSummaryUpdated(const FDBALobbyPlayerSummary& Summary)
{
	UpdatePlayerSummaryText(Summary);
}

void UDBAMainLobbyWidgetBase::HandleRecentMatchSummaryUpdated(const FDBALobbyRecentMatchSummary& Summary)
{
	UpdateRecentMatchSummaryText(Summary);
}

void UDBAMainLobbyWidgetBase::HandleCreateRoomClicked()
{
	BackendCreateRoom();
}

void UDBAMainLobbyWidgetBase::HandleRefreshRoomsClicked()
{
	BackendRefreshRoomList();
}

void UDBAMainLobbyWidgetBase::HandleRefreshMatchHistoryClicked()
{
	BackendRefreshMatchHistory();
}

void UDBAMainLobbyWidgetBase::HandleJoinRoomClicked()
{
	BackendJoinRoomFromInput();
}

void UDBAMainLobbyWidgetBase::HandleReadyClicked()
{
	const EDBALobbyBackendState State = WidgetController ? WidgetController->GetBackendState() : EDBALobbyBackendState::Idle;
	BackendSetReady(State != EDBALobbyBackendState::Ready);
}

void UDBAMainLobbyWidgetBase::HandleStartGameClicked()
{
	BackendStartRoom();
}

void UDBAMainLobbyWidgetBase::HandleStartMatchClicked()
{
	const FString Mode = MatchModeInput ? MatchModeInput->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Region = MatchRegionInput ? MatchRegionInput->GetText().ToString().TrimStartAndEnd() : FString();
	BackendStartMatchmaking(Mode.IsEmpty() ? TEXT("default") : Mode, Region.IsEmpty() ? TEXT("local") : Region);
}

void UDBAMainLobbyWidgetBase::HandleCancelMatchClicked()
{
	BackendCancelMatchmaking();
}

void UDBAMainLobbyWidgetBase::BindBackendUiControls()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!CreateRoomButton)
	{
		CreateRoomButton = Cast<UButton>(FindLobbyWidgetByNames(WidgetTree, { TEXT("CreateRoomButton"), TEXT("BtnCreateRoom"), TEXT("ButtonCreateRoom") }));
	}
	if (!RefreshRoomsButton)
	{
		RefreshRoomsButton = Cast<UButton>(FindLobbyWidgetByNames(WidgetTree, { TEXT("RefreshRoomsButton"), TEXT("BtnRefreshRooms"), TEXT("ButtonRefreshRooms") }));
	}
	if (!RefreshMatchHistoryButton)
	{
		RefreshMatchHistoryButton = Cast<UButton>(FindLobbyWidgetByNames(WidgetTree, { TEXT("RefreshMatchHistoryButton"), TEXT("BtnRefreshMatchHistory"), TEXT("ButtonRefreshMatchHistory"), TEXT("RefreshRecentMatchButton") }));
	}
	if (!JoinRoomButton)
	{
		JoinRoomButton = Cast<UButton>(FindLobbyWidgetByNames(WidgetTree, { TEXT("JoinRoomButton"), TEXT("BtnJoinRoom"), TEXT("ButtonJoinRoom") }));
	}
	if (!ReadyButton)
	{
		ReadyButton = Cast<UButton>(FindLobbyWidgetByNames(WidgetTree, { TEXT("ReadyButton"), TEXT("BtnReady"), TEXT("ButtonReady") }));
	}
	if (!StartGameButton)
	{
		StartGameButton = Cast<UButton>(FindLobbyWidgetByNames(WidgetTree, { TEXT("StartGameButton"), TEXT("BtnStartGame"), TEXT("ButtonStartGame"), TEXT("StartRoomButton") }));
	}
	if (!StartMatchButton)
	{
		StartMatchButton = Cast<UButton>(FindLobbyWidgetByNames(WidgetTree, { TEXT("StartMatchButton"), TEXT("BtnStartMatch"), TEXT("ButtonStartMatch"), TEXT("StartMatchmakingButton") }));
	}
	if (!CancelMatchButton)
	{
		CancelMatchButton = Cast<UButton>(FindLobbyWidgetByNames(WidgetTree, { TEXT("CancelMatchButton"), TEXT("BtnCancelMatch"), TEXT("ButtonCancelMatch"), TEXT("CancelMatchmakingButton") }));
	}

	if (!JoinRoomIdInput)
	{
		JoinRoomIdInput = Cast<UEditableTextBox>(FindLobbyWidgetByNames(WidgetTree, { TEXT("JoinRoomIdInput"), TEXT("InputJoinRoomId"), TEXT("RoomIdInput") }));
	}
	if (!MatchModeInput)
	{
		MatchModeInput = Cast<UEditableTextBox>(FindLobbyWidgetByNames(WidgetTree, { TEXT("MatchModeInput"), TEXT("InputMatchMode") }));
	}
	if (!MatchRegionInput)
	{
		MatchRegionInput = Cast<UEditableTextBox>(FindLobbyWidgetByNames(WidgetTree, { TEXT("MatchRegionInput"), TEXT("InputMatchRegion") }));
	}

	if (!PlayerNameText)
	{
		PlayerNameText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("PlayerNameText"), TEXT("TxtPlayerName"), TEXT("TextPlayerName") }));
	}
	if (!PlayerLevelText)
	{
		PlayerLevelText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("PlayerLevelText"), TEXT("TxtPlayerLevel"), TEXT("TextPlayerLevel") }));
	}
	if (!PlayerExperienceText)
	{
		PlayerExperienceText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("PlayerExperienceText"), TEXT("TxtPlayerExp"), TEXT("TextPlayerExp") }));
	}
	if (!PlayerGoldText)
	{
		PlayerGoldText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("PlayerGoldText"), TEXT("TxtPlayerGold"), TEXT("TextPlayerGold") }));
	}
	if (!PlayerTicketsText)
	{
		PlayerTicketsText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("PlayerTicketsText"), TEXT("TxtPlayerTickets"), TEXT("TextPlayerTickets") }));
	}
	if (!RecentMatchResultText)
	{
		RecentMatchResultText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("RecentMatchResultText"), TEXT("TxtRecentMatchResult"), TEXT("TextRecentMatchResult") }));
	}
	if (!RecentMatchMapText)
	{
		RecentMatchMapText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("RecentMatchMapText"), TEXT("TxtRecentMatchMap"), TEXT("TextRecentMatchMap") }));
	}
	if (!RecentMatchCombatText)
	{
		RecentMatchCombatText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("RecentMatchCombatText"), TEXT("TxtRecentMatchCombat"), TEXT("TextRecentMatchCombat") }));
	}
	if (!RecentMatchPlayedAtText)
	{
		RecentMatchPlayedAtText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("RecentMatchPlayedAtText"), TEXT("TxtRecentMatchPlayedAt"), TEXT("TextRecentMatchPlayedAt") }));
	}
	if (!RecentMatchRewardText)
	{
		RecentMatchRewardText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("RecentMatchRewardText"), TEXT("TxtRecentMatchReward"), TEXT("TextRecentMatchReward") }));
	}
	if (!BackendStateText)
	{
		BackendStateText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("BackendStateText"), TEXT("TxtBackendState"), TEXT("TextBackendState") }));
	}
	if (!BackendErrorText)
	{
		BackendErrorText = Cast<UTextBlock>(FindLobbyWidgetByNames(WidgetTree, { TEXT("BackendErrorText"), TEXT("TxtBackendError"), TEXT("TextBackendError"), TEXT("ErrorText") }));
	}

	if (CreateRoomButton)
	{
		CreateRoomButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleCreateRoomClicked);
		CreateRoomButton->OnClicked.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleCreateRoomClicked);
	}
	if (RefreshRoomsButton)
	{
		RefreshRoomsButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleRefreshRoomsClicked);
		RefreshRoomsButton->OnClicked.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleRefreshRoomsClicked);
	}
	if (RefreshMatchHistoryButton)
	{
		RefreshMatchHistoryButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleRefreshMatchHistoryClicked);
		RefreshMatchHistoryButton->OnClicked.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleRefreshMatchHistoryClicked);
	}
	if (JoinRoomButton)
	{
		JoinRoomButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleJoinRoomClicked);
		JoinRoomButton->OnClicked.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleJoinRoomClicked);
	}
	if (ReadyButton)
	{
		ReadyButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleReadyClicked);
		ReadyButton->OnClicked.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleReadyClicked);
	}
	if (StartGameButton)
	{
		StartGameButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleStartGameClicked);
		StartGameButton->OnClicked.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleStartGameClicked);
	}
	if (StartMatchButton)
	{
		StartMatchButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleStartMatchClicked);
		StartMatchButton->OnClicked.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleStartMatchClicked);
	}
	if (CancelMatchButton)
	{
		CancelMatchButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleCancelMatchClicked);
		CancelMatchButton->OnClicked.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleCancelMatchClicked);
	}
}

void UDBAMainLobbyWidgetBase::BindControllerDelegates()
{
	if (!WidgetController)
	{
		return;
	}

	WidgetController->OnBackendStateChanged.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleBackendStateChanged);
	WidgetController->OnBackendStateChanged.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleBackendStateChanged);
	WidgetController->OnBackendError.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleBackendError);
	WidgetController->OnBackendError.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleBackendError);
	WidgetController->OnPlayerSummaryUpdated.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandlePlayerSummaryUpdated);
	WidgetController->OnPlayerSummaryUpdated.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandlePlayerSummaryUpdated);
	WidgetController->OnRecentMatchSummaryUpdated.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleRecentMatchSummaryUpdated);
	WidgetController->OnRecentMatchSummaryUpdated.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleRecentMatchSummaryUpdated);
}

void UDBAMainLobbyWidgetBase::UnbindControllerDelegates()
{
	if (!WidgetController)
	{
		return;
	}

	WidgetController->OnBackendStateChanged.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleBackendStateChanged);
	WidgetController->OnBackendError.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleBackendError);
	WidgetController->OnPlayerSummaryUpdated.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandlePlayerSummaryUpdated);
	WidgetController->OnRecentMatchSummaryUpdated.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleRecentMatchSummaryUpdated);
}

void UDBAMainLobbyWidgetBase::UpdateBackendStateText(EDBALobbyBackendState NewState)
{
	if (BackendStateText)
	{
		BackendStateText->SetText(ToBackendStateText(NewState));
	}

	if (NewState != EDBALobbyBackendState::Error && BackendErrorText)
	{
		BackendErrorText->SetText(FText::GetEmpty());
		BackendErrorText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDBAMainLobbyWidgetBase::UpdateBackendButtonsState(EDBALobbyBackendState NewState)
{
	const bool bLoading = NewState == EDBALobbyBackendState::Loading || NewState == EDBALobbyBackendState::Connecting;
	const bool bInRoom = NewState == EDBALobbyBackendState::InRoom || NewState == EDBALobbyBackendState::Ready;
	const bool bMatching = NewState == EDBALobbyBackendState::Matching || NewState == EDBALobbyBackendState::MatchFound;

	if (CreateRoomButton)
	{
		CreateRoomButton->SetIsEnabled(!bLoading && !bMatching);
	}
	if (RefreshRoomsButton)
	{
		RefreshRoomsButton->SetIsEnabled(!bLoading);
	}
	if (JoinRoomButton)
	{
		JoinRoomButton->SetIsEnabled(!bLoading && !bMatching);
	}
	if (ReadyButton)
	{
		ReadyButton->SetIsEnabled(!bLoading && bInRoom);
	}
	if (StartGameButton)
	{
		StartGameButton->SetIsEnabled(!bLoading && bInRoom);
	}
	if (StartMatchButton)
	{
		StartMatchButton->SetIsEnabled(!bLoading && !bMatching);
	}
	if (CancelMatchButton)
	{
		CancelMatchButton->SetIsEnabled(!bLoading && bMatching);
	}
}

void UDBAMainLobbyWidgetBase::UpdatePlayerSummaryText(const FDBALobbyPlayerSummary& Summary)
{
	if (PlayerNameText)
	{
		const FString Name = Summary.DisplayName.IsEmpty() ? Summary.PlayerId : Summary.DisplayName;
		PlayerNameText->SetText(FText::FromString(Name));
	}
	if (PlayerLevelText)
	{
		PlayerLevelText->SetText(FText::AsNumber(Summary.Level));
	}
	if (PlayerExperienceText)
	{
		PlayerExperienceText->SetText(FText::AsNumber(Summary.Experience));
	}
	if (PlayerGoldText)
	{
		PlayerGoldText->SetText(FText::AsNumber(Summary.Gold));
	}
	if (PlayerTicketsText)
	{
		PlayerTicketsText->SetText(FText::AsNumber(Summary.Tickets));
	}
}

void UDBAMainLobbyWidgetBase::UpdateRecentMatchSummaryText(const FDBALobbyRecentMatchSummary& Summary)
{
	if (!Summary.bHasMatch)
	{
		const FText EmptyText = NSLOCTEXT("DBAMainLobby", "RecentMatchEmpty", "暂无最近战绩");
		if (RecentMatchResultText)
		{
			RecentMatchResultText->SetText(EmptyText);
		}
		if (RecentMatchMapText)
		{
			RecentMatchMapText->SetText(FText::GetEmpty());
		}
		if (RecentMatchCombatText)
		{
			RecentMatchCombatText->SetText(FText::GetEmpty());
		}
		if (RecentMatchPlayedAtText)
		{
			RecentMatchPlayedAtText->SetText(FText::GetEmpty());
		}
		if (RecentMatchRewardText)
		{
			RecentMatchRewardText->SetText(FText::GetEmpty());
		}
		return;
	}

	if (RecentMatchResultText)
	{
		const FString Result = Summary.Result.IsEmpty() ? TEXT("-") : Summary.Result;
		const FString WinnerTeam = Summary.WinnerTeam.IsEmpty() ? TEXT("-") : Summary.WinnerTeam;
		RecentMatchResultText->SetText(FText::FromString(FString::Printf(TEXT("%s / 胜方 %s / 得分 %d"), *Result, *WinnerTeam, Summary.Score)));
	}
	if (RecentMatchMapText)
	{
		const FString Mode = Summary.Mode.IsEmpty() ? TEXT("-") : Summary.Mode;
		const FString MapId = Summary.MapId.IsEmpty() ? TEXT("-") : Summary.MapId;
		RecentMatchMapText->SetText(FText::FromString(FString::Printf(TEXT("%s / %s"), *Mode, *MapId)));
	}
	if (RecentMatchCombatText)
	{
		const FString CombatText = Summary.CombatSummary.IsEmpty()
			? FString::Printf(TEXT("KDA %d/%d/%d"), Summary.Kills, Summary.Deaths, Summary.Assists)
			: Summary.CombatSummary;
		RecentMatchCombatText->SetText(FText::FromString(CombatText));
	}
	if (RecentMatchPlayedAtText)
	{
		RecentMatchPlayedAtText->SetText(FText::FromString(Summary.PlayedAtUtc));
	}
	if (RecentMatchRewardText)
	{
		const FString RewardsText = Summary.RewardSummary.IsEmpty()
			? FString::Printf(TEXT("金币 %+lld / 荣誉 %+lld"), Summary.CoinReward, Summary.HonorReward)
			: Summary.RewardSummary;
		RecentMatchRewardText->SetText(FText::FromString(FString::Printf(TEXT("EXP %+lld / %s"), Summary.ExpDelta, *RewardsText)));
	}
}
