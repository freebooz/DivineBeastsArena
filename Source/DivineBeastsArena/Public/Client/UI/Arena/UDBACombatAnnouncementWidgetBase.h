// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UUDBACombatAnnouncementWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACombatAnnouncementWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBACombatAnnouncementWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|CombatAnnouncement")
	void ShowAnnouncement(const FText& Text, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|CombatAnnouncement")
	void ClearAnnouncement();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|CombatAnnouncement", meta = (DisplayName = "On Announcement Shown"))
	void BP_OnAnnouncementShown(const FText& Text, float Duration);
};
