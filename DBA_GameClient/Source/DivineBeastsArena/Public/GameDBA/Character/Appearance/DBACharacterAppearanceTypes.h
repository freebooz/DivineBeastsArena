// Copyright Freebooz Games, Inc. All Rights Reserved.
// 角色外观持久化契约：仅保存稳定选项 ID，绝不保存客户端资产路径。

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBACharacterAppearanceTypes.generated.h"

UENUM(BlueprintType)
enum class EDBAAppearanceSlot : uint8
{
	Gender,
	BodyType,
	Face,
	Hair,
	HairColor,
	SkinColor,
	EyeColor,
	Marking,
	Horn,
	Ear,
	Tail,
	Equipment,
	Weapon,
	Skin
};

/**
 * 可持久化的角色外观。所有字段都是服务端可校验的稳定 ID；资源路径只存在于 Appearance Catalog DataAsset。
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBACharacterAppearance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName GenderId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName BodyTypeId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName FaceId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName HairId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName HairColorId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName SkinColorId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName EyeColorId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName MarkingId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName HornId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName EarId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName TailId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") TArray<FName> EquipmentVisualIds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName WeaponVisualId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Appearance") FName SkinId;

	bool operator==(const FDBACharacterAppearance& Other) const;
	bool operator!=(const FDBACharacterAppearance& Other) const { return !(*this == Other); }
	void GetSelectedOptionIds(TArray<FName>& OutOptionIds) const;
};

/** JSON 序列化边界，供 UE SaveGame/HTTP DTO Adapter 使用；不接收任意资产路径。 */
namespace DBACharacterAppearanceSerialization
{
	DIVINEBEASTSARENA_API bool ToJson(const FDBACharacterAppearance& Appearance, FString& OutJson);
	DIVINEBEASTSARENA_API bool FromJson(const FString& Json, FDBACharacterAppearance& OutAppearance);
}
