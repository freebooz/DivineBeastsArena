// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"
#include "DBACharacterCreateConfirmViewModel.generated.h"

struct FDBACharacterCreateDraft;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnCharacterCreateConfirmViewModelChanged);

/**
 * 角色创建确认页的只读显示投影。它只显示 Draft 的摘要和结构化错误，不持有 Token、HTTP DTO 或服务器实体；
 * 名称输入仍通过 Controller 写回唯一 Draft，最终名称唯一性及外观/构筑合法性始终由 CharacterService 裁决。
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterCreateConfirmViewModel final : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Confirm")
	const FString& GetCharacterName() const { return CharacterName; }
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Confirm")
	const FText& GetZodiacSummary() const { return ZodiacSummary; }
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Confirm")
	const FText& GetAppearanceSummary() const { return AppearanceSummary; }
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Confirm")
	const FText& GetElementSummary() const { return ElementSummary; }
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Confirm")
	const FText& GetFiveCampSummary() const { return FiveCampSummary; }
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Confirm")
	const FText& GetFixedBuildSummary() const { return FixedBuildSummary; }
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Confirm")
	const FText& GetAttributeSummary() const { return AttributeSummary; }
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Confirm")
	bool IsSubmitting() const { return bIsSubmitting; }
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Confirm")
	const FDBAApiError& GetLastError() const { return LastError; }

	void ApplyDraft(const FDBACharacterCreateDraft& Draft);
	void SetSubmitting(bool bInSubmitting);
	void SetError(const FDBAApiError& Error);
	void ClearError();

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterCreate|Confirm")
	FDBAOnCharacterCreateConfirmViewModelChanged OnChanged;

private:
	FString CharacterName;
	FText ZodiacSummary;
	FText AppearanceSummary;
	FText ElementSummary;
	FText FiveCampSummary;
	FText FixedBuildSummary;
	FText AttributeSummary;
	bool bIsSubmitting = false;
	FDBAApiError LastError;
};
