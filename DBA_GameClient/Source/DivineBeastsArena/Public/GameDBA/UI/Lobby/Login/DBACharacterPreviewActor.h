// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameFramework/Actor.h"
#include "DBACharacterPreviewActor.generated.h"

class USkeletalMeshComponent;
class UAnimationAsset;
class UPointLightComponent;
class USceneComponent;

UCLASS()
class DIVINEBEASTSARENA_API ADBACharacterPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ADBACharacterPreviewActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "DBA|Preview")
	void SetPreviewZodiac(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|Preview")
	void SetRotationSpeed(float InDegreesPerSecond);

	EDBAZodiac GetPreviewZodiac() const { return CurrentZodiac; }

protected:
	virtual void BeginPlay() override;

private:
	void ApplyPreviewAssets(EDBAZodiac Zodiac);

	UFUNCTION()
	void OnRep_CurrentZodiac();

private:
	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USceneComponent> PreviewRoot;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USkeletalMeshComponent> PreviewMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<UPointLightComponent> ZodiacTintLight;

	UPROPERTY(EditAnywhere, Category = "DBA|Preview")
	float RotationSpeedDegreesPerSecond = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentZodiac)
	EDBAZodiac CurrentZodiac = EDBAZodiac::Rat;
};
