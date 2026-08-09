// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterSelectViewModel.h"

void UDBACharacterSelectViewModel::ApplyRoster(const TArray<FDBACharacterSummary>& InCharacters, const FDBACharacterDetails* PreferredDetails)
{
	Characters = InCharacters;
	if (PreferredDetails)
	{
		SelectedDetails = *PreferredDetails;
		SelectedCharacterId = PreferredDetails->Summary.CharacterId;
	}
	else if (!Characters.ContainsByPredicate([this](const FDBACharacterSummary& Character) { return Character.CharacterId == SelectedCharacterId; }))
	{
		SelectedCharacterId = Characters.IsEmpty() ? FDBACharacterId() : Characters[0].CharacterId;
		SelectedDetails = FDBACharacterDetails();
	}
	OnChanged.Broadcast();
}

void UDBACharacterSelectViewModel::SelectCharacter(const FDBACharacterDetails& Details)
{
	SelectedDetails = Details;
	SelectedCharacterId = Details.Summary.CharacterId;
	OnChanged.Broadcast();
}

void UDBACharacterSelectViewModel::SetRosterLoading(const bool bLoading)
{
	if (bRosterLoading != bLoading) { bRosterLoading = bLoading; OnChanged.Broadcast(); }
}

void UDBACharacterSelectViewModel::SetPreviewLoading(const bool bLoading)
{
	if (bPreviewLoading != bLoading) { bPreviewLoading = bLoading; OnChanged.Broadcast(); }
}

void UDBACharacterSelectViewModel::SetDeleteConfirmationVisible(const bool bVisible)
{
	if (bDeleteConfirmationVisible != bVisible) { bDeleteConfirmationVisible = bVisible; OnChanged.Broadcast(); }
}

void UDBACharacterSelectViewModel::SetLastError(const FDBAApiError& Error)
{
	LastError = Error;
	OnChanged.Broadcast();
}

void UDBACharacterSelectViewModel::ClearError()
{
	if (LastError.IsError()) { LastError = FDBAApiError(); OnChanged.Broadcast(); }
}
