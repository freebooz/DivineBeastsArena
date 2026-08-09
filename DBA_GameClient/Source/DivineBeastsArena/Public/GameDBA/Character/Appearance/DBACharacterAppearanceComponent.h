// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"
#include "DBACharacterAppearanceComponent.generated.h"

struct FStreamableHandle;
class UDBAAppearanceCatalogDataAsset;
class UMaterialInterface;
class USkeletalMeshComponent;
struct FDBAAppearanceOptionDefinition;

DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnAppearanceApplied, const FDBACharacterAppearance& /* Appearance */, bool /* bAllOptionsResolved */);

/**
 * Preview Actor 与正式 Gameplay Character 共用的外观恢复组件。
 * 组件只消费稳定选项 ID，并从 Catalog DataAsset 解析软引用资源；Dedicated Server 不加载外观资源。
 */
UCLASS(ClassGroup = (DBA), BlueprintType, meta = (BlueprintSpawnableComponent))
class DIVINEBEASTSARENA_API UDBACharacterAppearanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBACharacterAppearanceComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 注入角色主体骨骼网格；未注入时会尝试从 Owner 查找首个 SkeletalMeshComponent。 */
	void SetBaseMeshComponent(USkeletalMeshComponent* InBaseMeshComponent);

	UFUNCTION(BlueprintCallable, Category = "DBA|Appearance")
	bool ApplyAppearance(EDBAZodiac Zodiac, const FDBACharacterAppearance& Appearance);

	UFUNCTION(BlueprintCallable, Category = "DBA|Appearance")
	void Reset();

	/** 重新请求当前外观的 Catalog 和已选资源。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Appearance")
	bool AsyncLoad();

	/** 对已加载定义应用模块部件；只接受 Catalog 已验证的 OptionId。 */
	bool ApplyPart(const FDBAAppearanceOptionDefinition& Definition);

	/** 应用已加载材质及颜色参数。 */
	void ApplyMaterialParameters(const FDBAAppearanceOptionDefinition& Definition);

	const FDBACharacterAppearance& GetCurrentAppearance() const { return CurrentAppearance; }
	EDBAZodiac GetCurrentZodiac() const { return CurrentZodiac; }
	FDBAOnAppearanceApplied OnAppearanceApplied;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Appearance")
	TSoftObjectPtr<UDBAAppearanceCatalogDataAsset> AppearanceCatalog;

private:
	struct FRequestedOption
	{
		EDBAAppearanceSlot Slot = EDBAAppearanceSlot::Hair;
		FName OptionId;
	};

	USkeletalMeshComponent* ResolveBaseMeshComponent();
	void CollectRequestedOptions(TArray<FRequestedOption>& OutOptions) const;
	void BeginAsyncApply(uint32 RequestVersion);
	void ApplyResolvedOptions(uint32 RequestVersion);
	const FDBAAppearanceOptionDefinition* ResolveDefinition(const FRequestedOption& RequestedOption, bool& bOutUsedFallback) const;
	void RestoreBaseMaterials();
	void ClearModularParts();
	bool IsDedicatedServer() const;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> BaseMeshComponent;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<USkeletalMeshComponent>> ModularPartComponents;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UMaterialInterface>> OriginalBaseMaterials;

	FDBACharacterAppearance CurrentAppearance;
	EDBAZodiac CurrentZodiac = EDBAZodiac::None;
	uint32 AppearanceRequestVersion = 0;
	TSharedPtr<FStreamableHandle> ActiveLoadHandle;
};
