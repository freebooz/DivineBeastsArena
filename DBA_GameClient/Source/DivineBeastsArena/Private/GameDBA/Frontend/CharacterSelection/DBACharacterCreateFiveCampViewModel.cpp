// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateFiveCampViewModel.h"

#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"

void UDBACharacterCreateFiveCampViewModel::ApplyDraft(const FDBACharacterCreateDraft& Draft)
{
	// Draft 是跨 Screen 的唯一选择状态；ViewModel 不反向保存一份可被 Widget 修改的五营值。
	SelectedFiveCamp = Draft.FiveCampType;
	RefreshSelection();
}

void UDBACharacterCreateFiveCampViewModel::ApplyFiveCampCards(const TArray<FDBACharacterCreateFiveCampCardModel>& InCards)
{
	FiveCampCards = InCards;
	RefreshSelection();
}

void UDBACharacterCreateFiveCampViewModel::SetValidationMessage(const FText& Message)
{
	ValidationMessage = Message;
	OnChanged.Broadcast();
}

void UDBACharacterCreateFiveCampViewModel::RefreshSelection()
{
	SelectedCard = FDBACharacterCreateFiveCampCardModel();
	for (FDBACharacterCreateFiveCampCardModel& Card : FiveCampCards)
	{
		Card.bIsSelected = Card.FiveCamp == SelectedFiveCamp;
		if (Card.bIsSelected)
		{
			SelectedCard = Card;
		}
	}
	OnChanged.Broadcast();
}
