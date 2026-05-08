// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h"
#include "GameDBA/Core/DBALogChannels.h"

UDBACharacterSelectFlowWidgetBase::UDBACharacterSelectFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACharacterSelectFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBACharacterSelectFlowWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBACharacterSelectFlowWidgetBase::UpdateCharacters(const TArray<FDBACharacterSummary>& Characters)
{
	CurrentCharacters = Characters;
	BP_OnCharactersUpdated(Characters);
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] 更新角色列表: %d"), Characters.Num());
}

void UDBACharacterSelectFlowWidgetBase::SelectCharacter(const FDBACharacterId& CharacterId)
{
	SelectedCharacterId = CharacterId;
	BP_OnCharacterSelected(CharacterId);
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] 选择角色: %s"), *CharacterId.ToString());
}
