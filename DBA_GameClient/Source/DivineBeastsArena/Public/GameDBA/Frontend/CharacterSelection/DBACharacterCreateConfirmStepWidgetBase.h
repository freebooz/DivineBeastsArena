// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterCreateConfirmStepWidgetBase.generated.h"

class UButton;
class UEditableTextBox;
class UDBACharacterCreateWidgetController;
class UDBACharacterCreateConfirmViewModel;

/**
 * WBP_DBA_CharacterCreate_ConfirmStep 的 C++ 父类。负责名称输入、Create/Back/取消意图转发及 ViewModel 订阅；
 * 它不访问 ApiClient、不保管幂等键、不保存角色权威数据，也不自行创建或切换任何 Screen。
 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateConfirmStepWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Confirm")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Confirm")
	void SubmitCreate();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Confirm")
	void BackStep();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Confirm")
	void CancelSubmission();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleNameChanged(const FText& NewName);
	UFUNCTION()
	void HandleCreateClicked();
	UFUNCTION()
	void HandleBackClicked();
	UFUNCTION()
	void HandleCancelClicked();
	UFUNCTION()
	void HandleViewModelChanged();

	/** 蓝图只根据投影刷新摘要、Loading、错误横幅和最终 3D 预览占位表现。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate|Confirm")
	void BP_OnConfirmViewModelChanged(UDBACharacterCreateConfirmViewModel* InViewModel);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate|Confirm")
	TObjectPtr<UEditableTextBox> CharacterNameInput;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate|Confirm")
	TObjectPtr<UButton> CreateButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate|Confirm")
	TObjectPtr<UButton> BackButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate|Confirm")
	TObjectPtr<UButton> CancelButton;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;
	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateConfirmViewModel> ViewModel;
};
