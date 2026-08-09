// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Widgets/Common/DBAModalDialogWidgetBase.h"

void UDBAModalDialogWidgetBase::SetDialogContent(const FText& InTitle, const FText& InBody)
{
	Title = InTitle;
	Body = InBody;
	BP_OnDialogContentChanged(Title, Body);
}
