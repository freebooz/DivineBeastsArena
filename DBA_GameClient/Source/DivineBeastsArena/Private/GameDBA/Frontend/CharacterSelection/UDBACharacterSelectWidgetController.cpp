// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/UDBACharacterSelectWidgetController.h"

#include "GameDBA/Frontend/Character/DBACharacterRosterSubsystem.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterSelectViewModel.h"
#include "GameDBA/Frontend/Core/DBAFrontendErrorMapper.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewSubsystem.h"
#include "Engine/GameInstance.h"

UDBACharacterSelectWidgetController::UDBACharacterSelectWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACharacterSelectWidgetController::BindLoginFlow()
{
	if (!ViewModel) ViewModel = NewObject<UDBACharacterSelectViewModel>(this);
	UDBAFrontendFlowSubsystem* Flow = GetLoginFlow();
	UDBACharacterRosterSubsystem* NewRoster = GetRoster();
	UDBACharacterPreviewSubsystem* NewPreview = GetPreviewSubsystem();
	if (!Flow || !NewRoster || !NewPreview)
	{
		PublishError(FDBAOperationResult::Failure(EDBAErrorCode::ServiceUnavailable, TEXT("角色选择服务尚未就绪。")));
		return;
	}

	UnbindServices();
	Roster = NewRoster;
	PreviewSubsystem = NewPreview;
	RosterChangedHandle = NewRoster->OnCharacterRosterChanged().AddUObject(this, &UDBACharacterSelectWidgetController::HandleRosterChanged);
	PreviewResolvedHandle = NewPreview->OnCharacterPreviewResolved.AddUObject(this, &UDBACharacterSelectWidgetController::HandlePreviewResolved);
	Flow->OnCharactersLoaded.AddDynamic(this, &UDBACharacterSelectWidgetController::HandleCharactersLoaded);
	Flow->OnFrontendStateChanged.AddDynamic(this, &UDBACharacterSelectWidgetController::HandleFlowStateChanged);
	HandleRosterChanged(NewRoster->GetCachedCharacters());
}

void UDBACharacterSelectWidgetController::SelectCharacter(const FDBACharacterId& CharacterId)
{
	ApplySelection(CharacterId);
}

void UDBACharacterSelectWidgetController::EnterGame()
{
	if (!ViewModel || !ViewModel->CanEnterGame()) return;
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow()) Flow->SubmitCharacterSelection(ViewModel->GetSelectedCharacterId());
}

void UDBACharacterSelectWidgetController::CreateCharacter()
{
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow()) Flow->EnterCharacterCreate();
}

void UDBACharacterSelectWidgetController::Refresh()
{
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow())
	{
		if (ViewModel) ViewModel->SetRosterLoading(true);
		Flow->RefreshCharacterList();
	}
}

void UDBACharacterSelectWidgetController::RequestDeleteSelectedCharacter()
{
	if (!ViewModel || !ViewModel->GetSelectedCharacterId().IsValid()) return;
	const FDBACharacterSummary* Summary = ViewModel->GetCharacters().FindByPredicate([this](const FDBACharacterSummary& Character) { return Character.CharacterId == ViewModel->GetSelectedCharacterId(); });
	if (!Summary) return;
	ViewModel->SetDeleteConfirmationVisible(true);
	OnDeleteConfirmationRequested.Broadcast(*Summary);
}

void UDBACharacterSelectWidgetController::ConfirmDelete()
{
	if (!ViewModel || !ViewModel->IsDeleteConfirmationVisible()) return;
	const FDBACharacterId DeletedId = ViewModel->GetSelectedCharacterId();
	ViewModel->SetDeleteConfirmationVisible(false);
	if (UDBACharacterRosterSubsystem* CurrentRoster = GetRoster())
	{
		ViewModel->SetRosterLoading(true);
		CurrentRoster->DeleteCharacter(DeletedId, [this, DeletedId](const FDBAOperationResult& Result)
		{
			if (!ViewModel) return;
			ViewModel->SetRosterLoading(false);
			if (!Result.bSuccess) { PublishError(Result); return; }
			const TArray<FDBACharacterSummary>& Characters = GetRoster()->GetCachedCharacters();
			const FDBACharacterSummary* Next = Characters.FindByPredicate([DeletedId](const FDBACharacterSummary& Character) { return Character.CharacterId != DeletedId; });
			if (Next) ApplySelection(Next->CharacterId); else HandleRosterChanged(Characters);
		});
	}
}

void UDBACharacterSelectWidgetController::CancelDelete()
{
	if (ViewModel) ViewModel->SetDeleteConfirmationVisible(false);
}

void UDBACharacterSelectWidgetController::BackToServerSelect()
{
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow()) Flow->BeginServerSelection();
}

void UDBACharacterSelectWidgetController::HandleCharactersLoaded(const TArray<FDBACharacterSummary>& Characters)
{
	HandleRosterChanged(Characters);
}

void UDBACharacterSelectWidgetController::HandleRosterChanged(const TArray<FDBACharacterSummary>& Characters)
{
	if (!ViewModel) return;
	ViewModel->SetRosterLoading(false);
	const FDBACharacterDetails* Details = nullptr;
	if (UDBACharacterRosterSubsystem* CurrentRoster = GetRoster())
	{
		const FDBACharacterId Desired = ViewModel->GetSelectedCharacterId().IsValid() ? ViewModel->GetSelectedCharacterId() : (Characters.IsEmpty() ? FDBACharacterId() : Characters[0].CharacterId);
		Details = CurrentRoster->FindCachedCharacter(Desired);
	}
	ViewModel->ApplyRoster(Characters, Details);
	OnCharactersChanged.Broadcast(Characters);
	if (ViewModel->GetSelectedCharacterId().IsValid()) ApplySelection(ViewModel->GetSelectedCharacterId());
}

void UDBACharacterSelectWidgetController::ApplySelection(const FDBACharacterId& CharacterId)
{
	UDBACharacterRosterSubsystem* CurrentRoster = GetRoster();
	UDBACharacterPreviewSubsystem* Preview = GetPreviewSubsystem();
	if (!ViewModel || !CurrentRoster || !Preview) return;
	const FDBACharacterDetails* Details = CurrentRoster->FindCachedCharacter(CharacterId);
	if (!Details) return;
	ViewModel->SelectCharacter(*Details);
	ViewModel->SetPreviewLoading(true);
	if (!Preview->SelectCharacter(Details->Summary.Zodiac, Details->Appearance)) ViewModel->SetPreviewLoading(false);
}

void UDBACharacterSelectWidgetController::HandlePreviewResolved(const EDBAZodiac Zodiac, const bool bSuccess)
{
	if (!ViewModel || ViewModel->GetSelectedDetails().Summary.Zodiac != Zodiac) return;
	ViewModel->SetPreviewLoading(false);
	if (!bSuccess) PublishError(FDBAOperationResult::Failure(EDBAErrorCode::LoadFailure, TEXT("角色预览资源加载失败。")));
}

void UDBACharacterSelectWidgetController::HandleFlowStateChanged(const EDBAFrontendState PreviousState, const EDBAFrontendState NewState)
{
	if (!ViewModel) return;
	ViewModel->SetRosterLoading(NewState == EDBAFrontendState::CharacterRosterLoading);
	if (PreviousState == EDBAFrontendState::CharacterRosterLoading && NewState == EDBAFrontendState::CharacterSelect) ViewModel->ClearError();
}

void UDBACharacterSelectWidgetController::PublishError(const FDBAOperationResult& Result)
{
	if (!ViewModel) return;
	FDBAApiError Error = Result.ApiError;
	if (!Error.IsError()) Error = UDBAFrontendErrorMapper::FromLegacyMessage(Result.ErrorMessage.IsEmpty() ? TEXT("角色选择操作失败。") : Result.ErrorMessage);
	ViewModel->SetLastError(Error);
	OnCharacterSelectError.Broadcast(Error);
}

void UDBACharacterSelectWidgetController::UnbindServices()
{
	if (Roster.IsValid() && RosterChangedHandle.IsValid()) Roster->OnCharacterRosterChanged().Remove(RosterChangedHandle);
	if (PreviewSubsystem.IsValid() && PreviewResolvedHandle.IsValid()) PreviewSubsystem->OnCharacterPreviewResolved.Remove(PreviewResolvedHandle);
	if (UDBAFrontendFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->OnCharactersLoaded.RemoveDynamic(this, &UDBACharacterSelectWidgetController::HandleCharactersLoaded);
		Flow->OnFrontendStateChanged.RemoveDynamic(this, &UDBACharacterSelectWidgetController::HandleFlowStateChanged);
	}
	Roster.Reset(); PreviewSubsystem.Reset(); RosterChangedHandle.Reset(); PreviewResolvedHandle.Reset();
}

UDBAFrontendFlowSubsystem* UDBACharacterSelectWidgetController::GetLoginFlow() const
{
	return GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr;
}

UDBACharacterRosterSubsystem* UDBACharacterSelectWidgetController::GetRoster() const
{
	return GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>() : nullptr;
}

UDBACharacterPreviewSubsystem* UDBACharacterSelectWidgetController::GetPreviewSubsystem() const
{
	return GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UDBACharacterPreviewSubsystem>() : nullptr;
}
