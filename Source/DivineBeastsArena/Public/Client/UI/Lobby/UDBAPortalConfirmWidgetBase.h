// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBAPortalConfirmWidgetBase.generated.h"

/**
 * DBAPortalConfirmWidgetBase
 *
 * 传送门确认 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAPortalConfirmWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAPortalConfirmWidgetBase(const FObjectInitializer& ObjectInitializer);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortalConfirmed, FName, DestinationId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPortalCancelled);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|PortalConfirm")
	void ShowConfirm(FName DestinationId, const FText& DestinationName, const FText& DestinationDescription, bool bCanTeleport, const FText& ConditionText);

	UFUNCTION(BlueprintCallable, Category = "DBA|PortalConfirm")
	void ConfirmTeleport();

	UFUNCTION(BlueprintCallable, Category = "DBA|PortalConfirm")
	void CancelTeleport();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|PortalConfirm", meta = (DisplayName = "On Show Confirm"))
	void BP_OnShowConfirm(const FText& DestinationName, const FText& DestinationDescription, bool bCanTeleport, const FText& ConditionText);

public:
	UPROPERTY(BlueprintAssignable, Category = "DBA|PortalConfirm")
	FOnPortalConfirmed OnPortalConfirmedEvent;

	UPROPERTY(BlueprintAssignable, Category = "DBA|PortalConfirm")
	FOnPortalCancelled OnPortalCancelledEvent;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|PortalConfirm")
	FName DestinationId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|PortalConfirm")
	FText CachedDestinationName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|PortalConfirm")
	FText CachedDestinationDescription;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|PortalConfirm")
	bool CachedCanTeleport;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|PortalConfirm")
	FText CachedConditionText;
};
