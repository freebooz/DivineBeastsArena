// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetBase.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAFixedSkillGroupPreviewWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAFixedSkillGroupPreviewWidgetBase : public UDBAWidgetBase
{
	GENERATED_BODY()

public:
	UDBAFixedSkillGroupPreviewWidgetBase(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|SkillGroupPreview")
	void SetZodiacAndElement(EDBAZodiac Zodiac, EDBAElement Element);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|SkillGroupPreview", meta = (DisplayName = "On Update Skill Group Preview"))
	void BP_OnUpdateSkillGroupPreview(
		EDBAZodiac Zodiac,
		EDBAElement Element,
		const FText& PassiveName,
		const FText& Skill01Name,
		const FText& Skill02Name,
		const FText& Skill03Name,
		const FText& Skill04Name,
		const FText& UltimateName,
		int32 ResonanceLevel,
		const FText& ResonanceDescription
	);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|SkillGroupPreview")
	EDBAZodiac CurrentZodiac;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|SkillGroupPreview")
	EDBAElement CurrentElement;
};
