// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/UI/Framework/DBACommonScreenBase.h"
#include "DBACommonModalBase.generated.h"

/** Modal presentation base. It does not alter Flow state and is always mounted in ModalLayer. */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACommonModalBase : public UDBACommonScreenBase
{
	GENERATED_BODY()

public:
	UDBACommonModalBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category="DBA|UI|Modal")
	void Dismiss();
};
