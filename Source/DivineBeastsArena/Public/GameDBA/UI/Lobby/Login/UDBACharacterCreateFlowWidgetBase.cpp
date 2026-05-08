// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.h"
#include "GameDBA/Core/DBALogChannels.h"

UDBACharacterCreateFlowWidgetBase::UDBACharacterCreateFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACharacterCreateFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBACharacterCreateFlowWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBACharacterCreateFlowWidgetBase::SetCharacterName(const FString& Name)
{
	CharacterName = Name;
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 设置名称: %s"), *Name);
}

void UDBACharacterCreateFlowWidgetBase::SetZodiac(EDBAZodiac Zodiac)
{
	SelectedZodiac = Zodiac;
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 设置生肖: %d"), static_cast<int32>(Zodiac));
}

void UDBACharacterCreateFlowWidgetBase::SetElement(EDBAElement Element)
{
	SelectedElement = Element;
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 设置元素: %d"), static_cast<int32>(Element));
}

void UDBACharacterCreateFlowWidgetBase::SetFiveCamp(EDBAFiveCamp FiveCamp)
{
	SelectedFiveCamp = FiveCamp;
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 设置阵营: %d"), static_cast<int32>(FiveCamp));
}

void UDBACharacterCreateFlowWidgetBase::Submit()
{
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 提交创建"));
}
