// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"
#include "GameDBA/Frontend/Preview/DBAFiveCampPreviewTheme.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DBACharacterPreviewSubsystem.generated.h"

class ADBACharacterPresentationActor;
class ADBACharacterPreviewActor;
class ADBACharacterPreviewStage;
class APlayerController;
class UDBAZodiacHeroDataAsset;

/** 可测试的代次闸门：仅最后一次异步选择可以写入当前预览。 */
struct FDBACharacterPreviewRequestGate
{
	uint32 BeginRequest() { return ++Generation; }
	void Invalidate() { ++Generation; }
	bool IsCurrent(const uint32 Candidate) const { return Candidate != 0 && Candidate == Generation; }
	uint32 GetGeneration() const { return Generation; }

private:
	uint32 Generation = 0;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnCharacterPreviewResolved, EDBAZodiac /* Zodiac */, bool /* bSuccess */);

/**
 * 前台唯一角色预览入口。它驱动按需加载、舞台/相机生命周期与输入交互，不让 Widget 直接生成 Actor 或加载资源。
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBACharacterPreviewSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Dedicated Server 没有前台角色预览职责；在子系统创建阶段直接排除，
	 * 避免仅依赖运行时分支而意外持有前台资源或注册预览回调。
	 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	bool SelectZodiac(EDBAZodiac Zodiac, const FDBACharacterAppearance& Appearance);
	bool SelectCharacter(EDBAZodiac Zodiac, const FDBACharacterAppearance& Appearance) { return SelectZodiac(Zodiac, Appearance); }
	void Rotate(float DeltaYawDegrees);
	void Zoom(float DeltaDistance);
	void ResetCamera();
	void ActivateCamera(APlayerController* PlayerController, float BlendTime = 0.0f);
	void PlaySelect();
	void PlayIdleVariation();
	/** 将五营选择的已解析表现主题转交给 PreviewStage；不会写入角色外观或对局 TeamId。 */
	void ApplyFiveCampTheme(const FDBAFiveCampPreviewTheme& Theme);
	void ClearFiveCampTheme();
	void ReleasePreview();

	uint32 GetRequestGeneration() const { return RequestGate.GetGeneration(); }
	FDBAOnCharacterPreviewResolved OnCharacterPreviewResolved;

private:
	bool IsDedicatedServer() const;
	ADBACharacterPreviewStage* ResolvePreviewStage() const;
	ADBACharacterPresentationActor* ResolveLegacyPresentationStage() const;
	void OnZodiacAssetLoaded(EDBAZodiac Zodiac, UDBAZodiacHeroDataAsset* HeroData, FDBACharacterAppearance Appearance, uint32 RequestGeneration);

	FDBACharacterPreviewRequestGate RequestGate;
	EDBAZodiac ActiveZodiac = EDBAZodiac::None;
	TWeakObjectPtr<ADBACharacterPreviewActor> ActivePreviewActor;
};
