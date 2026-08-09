// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACommonScreenBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnUIBackRequested);

/**
 * Small common screen base for new UI. Existing UUserWidget screens remain compatible while they are migrated.
 * It intentionally contains presentation and input routing only; Flow owns business navigation.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACommonScreenBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBACommonScreenBase(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category="DBA|UI|Input")
	FDBAOnUIBackRequested OnBackRequested;

	UFUNCTION(BlueprintCallable, Category="DBA|UI|Input")
	virtual void RequestBack();

	UFUNCTION(BlueprintCallable, Category="DBA|UI|Input")
	void SetPreferredFocus(UWidget* InWidget);

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DBA|UI|Input")
	bool bConsumesBackInput = true;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> PreferredFocusWidget;
};
