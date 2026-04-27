// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Lobby/DBACharacterRosterSubsystem.h"
#include "Common/DBALogChannels.h"

UDBACharacterRosterSubsystem::UDBACharacterRosterSubsystem()
{
}

void UDBACharacterRosterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogDBAUI, Log, TEXT("[UDBACharacterRosterSubsystem] 初始化完成"));
}

void UDBACharacterRosterSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UDBACharacterRosterSubsystem::HasCharacter(FName CharacterId) const
{
	return OwnedCharacters.Contains(CharacterId);
}

TArray<FName> UDBACharacterRosterSubsystem::GetOwnedCharacters() const
{
	return OwnedCharacters;
}

void UDBACharacterRosterSubsystem::AddCharacter(FName CharacterId)
{
	if (!OwnedCharacters.Contains(CharacterId))
	{
		OwnedCharacters.Add(CharacterId);
		UE_LOG(LogDBAUI, Log, TEXT("[UDBACharacterRosterSubsystem] 添加角色: %s"), *CharacterId.ToString());
	}
}

int32 UDBACharacterRosterSubsystem::GetCharacterCount() const
{
	return OwnedCharacters.Num();
}
