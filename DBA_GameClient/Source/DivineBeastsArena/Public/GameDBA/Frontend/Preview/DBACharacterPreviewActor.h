// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"
#include "GameFramework/Actor.h"
#include "DBACharacterPreviewActor.generated.h"

class UAudioComponent;
class UDBACharacterAppearanceComponent;
class UDBAZodiacHeroDataAsset;
class UNiagaraComponent;
class USceneComponent;
class USkeletalMeshComponent;
struct FStreamableHandle;

/** 只承载展示网格、动画、外观、预览特效和音频的轻量前台 Actor；不包含 Gameplay、GAS、复制或 AI。 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API ADBACharacterPreviewActor final : public AActor
{
	GENERATED_BODY()

public:
	ADBACharacterPreviewActor();

	bool ApplyZodiacData(const UDBAZodiacHeroDataAsset& HeroData, const FDBACharacterAppearance& Appearance, uint32 RequestGeneration);
	void Rotate(float DeltaYawDegrees);
	void PlaySelect();
	void PlayIdleVariation();
	void ReleasePreviewResources();

	UDBACharacterAppearanceComponent* GetAppearanceComponent() const { return AppearanceComponent; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void CompleteVisualLoad(uint32 RequestGeneration, const UDBAZodiacHeroDataAsset* HeroData, FDBACharacterAppearance Appearance);
	bool IsCurrentRequest(uint32 RequestGeneration) const;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USkeletalMeshComponent> PreviewMesh;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<UDBACharacterAppearanceComponent> AppearanceComponent;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<UNiagaraComponent> PreviewVfx;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<UAudioComponent> PreviewAudio;

	uint32 ActiveRequestGeneration = 0;
	TSharedPtr<FStreamableHandle> ActiveVisualLoadHandle;
	TWeakObjectPtr<const UDBAZodiacHeroDataAsset> ActiveHeroData;
};
