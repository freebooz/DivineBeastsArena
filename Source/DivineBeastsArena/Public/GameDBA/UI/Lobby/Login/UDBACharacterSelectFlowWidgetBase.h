// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "UDBACharacterSelectFlowWidgetBase.generated.h"

/**
 * UDBACharacterSelectFlowWidgetBase
 * 角色选择流程Widget基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterSelectFlowWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBACharacterSelectFlowWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

public:
	/** 更新角色列表 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	virtual void UpdateCharacters(const TArray<FDBACharacterSummary>& Characters);

	/** 选择角色 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	virtual void SelectCharacter(const FDBACharacterId& CharacterId);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterSelect", meta = (DisplayName = "On Characters Updated"))
	void BP_OnCharactersUpdated(const TArray<FDBACharacterSummary>& Characters);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterSelect", meta = (DisplayName = "On Character Selected"))
	void BP_OnCharacterSelected(const FDBACharacterId& CharacterId);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterSelect")
	TArray<FDBACharacterSummary> CurrentCharacters;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterSelect")
	FDBACharacterId SelectedCharacterId;
};
