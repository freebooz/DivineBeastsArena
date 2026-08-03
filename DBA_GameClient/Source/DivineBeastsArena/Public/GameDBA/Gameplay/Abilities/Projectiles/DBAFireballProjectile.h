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
#include "GameDBA/Gameplay/Abilities/Projectiles/DBASkillProjectileBase.h"
#include "DBAFireballProjectile.generated.h"

class UMaterialInterface;
class UAudioComponent;
class UPointLightComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAFireballProjectile : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAFireballProjectile();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnProjectileHit(AActor* HitActor, FVector HitLocation) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FireballCore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> FireballLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> FireballLoopAudio;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Fireball")
	TObjectPtr<UMaterialInterface> FireballCoreMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Fireball")
	FLinearColor FireballColor = FLinearColor(1.0f, 0.22f, 0.02f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Fireball")
	float FireballPulseSpeed = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Fireball")
	float FireballPulseAmount = 0.12f;

private:
	float AgeSeconds = 0.0f;
};
