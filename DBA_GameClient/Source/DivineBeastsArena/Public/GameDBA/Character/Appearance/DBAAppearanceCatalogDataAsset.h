// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Data/DBADataAssetBase.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
#include "DBAAppearanceCatalogDataAsset.generated.h"

class UAnimInstance;
class UMaterialInterface;
class USkeletalMesh;

/** 单个稳定外观 ID 对应的资源定义；资产路径只允许存在于此配置资产。 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAAppearanceOptionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable, Category = "DBA|Appearance")
	FName OptionId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance")
	EDBAAppearanceSlot Slot = EDBAAppearanceSlot::Hair;

	/** 空数组表示所有生肖都可使用；非空时按生肖白名单校验。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance")
	TArray<EDBAZodiac> AllowedZodiacs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance")
	bool bFallbackForSlot = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance|Mesh")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance|Mesh")
	FName AttachSocket;

	/** 仅同骨架模块使用 LeaderPose；异骨架必须配置 CopyPose AnimBP，不能盲目 MeshMerge。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance|Mesh")
	bool bUseLeaderPose = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance|Mesh")
	TSoftClassPtr<UAnimInstance> CopyPoseAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance|Material", meta = (ClampMin = "0"))
	int32 MaterialSlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance|Material")
	TArray<TSoftObjectPtr<UMaterialInterface>> MaterialOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance|Material")
	TMap<FName, FLinearColor> MaterialVectorParameters;
};

/** 外观选项唯一目录，供服务端校验、UI 生成和客户端资源恢复共同使用。 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAAppearanceCatalogDataAsset : public UDBADataAssetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Appearance")
	TArray<FDBAAppearanceOptionDefinition> Options;

	const FDBAAppearanceOptionDefinition* FindOption(FName OptionId) const;
	const FDBAAppearanceOptionDefinition* FindFallback(EDBAAppearanceSlot Slot, EDBAZodiac Zodiac) const;
	bool IsOptionAllowed(const FDBAAppearanceOptionDefinition& Definition, EDBAZodiac Zodiac) const;

	/** UI 只能通过此数据定义生成指定生肖和槽位的控件，不得维护本地选项表。 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Appearance")
	void GetAvailableOptionIds(EDBAZodiac Zodiac, EDBAAppearanceSlot Slot, TArray<FName>& OutOptionIds) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
