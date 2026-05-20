// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetController.h"
#include "GameDBA/UI/Lobby/UDBAPartyPanelWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAQueueModeSelectWidgetBase.h"

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
			return NSLOCTEXT("DBAMainLobby", "BackendStateIdle", "Idle");
		case EDBALobbyBackendState::Loading:
			return NSLOCTEXT("DBAMainLobby", "BackendStateLoading", "Loading");
		case EDBALobbyBackendState::InRoom:
			return NSLOCTEXT("DBAMainLobby", "BackendStateInRoom", "In Room");
		case EDBALobbyBackendState::Ready:
			return NSLOCTEXT("DBAMainLobby", "BackendStateReady", "Ready");
		case EDBALobbyBackendState::Matching:
			return NSLOCTEXT("DBAMainLobby", "BackendStateMatching", "Matching");
		case EDBALobbyBackendState::MatchFound:
			return NSLOCTEXT("DBAMainLobby", "BackendStateMatchFound", "Match Found");
		case EDBALobbyBackendState::Connecting:
			return NSLOCTEXT("DBAMainLobby", "BackendStateConnecting", "Connecting");
		case EDBALobbyBackendState::Error:
			return NSLOCTEXT("DBAMainLobby", "BackendStateError", "Error");
		default:
			return NSLOCTEXT("DBAMainLobby", "BackendStateUnknown", "Unknown");
		}
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
}

void UDBAMainLobbyWidgetBase::NavigateToPractice()
{
}

void UDBAMainLobbyWidgetBase::OpenQueueModeSelect()
{
	if (QueueModeSelect)
	{
		QueueModeSelect->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDBAMainLobbyWidgetBase::OpenFriendList()
{
}

void UDBAMainLobbyWidgetBase::OpenSettings()
{
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

void UDBAMainLobbyWidgetBase::HandleCreateRoomClicked()
{
	BackendCreateRoom();
}

void UDBAMainLobbyWidgetBase::HandleRefreshRoomsClicked()
{
	BackendRefreshRoomList();
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
