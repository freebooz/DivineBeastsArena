// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Characters/Monster/DBALobbyTrainingMonster.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "GameDBA/Presentation/VFX/Feedback/DBALobbyFloatingDamageComponent.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Presentation/Visual/DBAZodiacVisualDeveloperSettings.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/UI/Widgets/Lobby/DBALobbyMonsterHealthBarWidget.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

namespace
{
	const FLinearColor LobbyMonsterTints[] = {
		FLinearColor(0.95f, 0.12f, 0.08f, 1.0f),
		FLinearColor(0.10f, 0.45f, 1.00f, 1.0f),
		FLinearColor(0.10f, 0.80f, 0.28f, 1.0f),
		FLinearColor(1.00f, 0.72f, 0.10f, 1.0f)
	};
}

ADBALobbyTrainingMonster::ADBALobbyTrainingMonster()
{
	MonsterType = TEXT("LobbyTrainingMonster");
	// P2-3 改造：MaxHealth/CurrentHealth 已迁移到 BattleAttributeSet，由 DBABattleAttributeDeveloperSettings
	// 的 DefaultBattleAttributeDefaults 数据资产驱动，不再在子类构造函数中硬编码生命值（符合 DBA.DataAsset.NoHardcoding 策略）。
	AIControllerClass = nullptr;
	AutoPossessAI = EAutoPossessAI::Disabled;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawSize(FVector2D(48.0f, 5.0f));
	HealthBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarComponent->SetWidgetClass(UDBALobbyMonsterHealthBarWidget::StaticClass());

	FloatingDamageComponent = CreateDefaultSubobject<UDBALobbyFloatingDamageComponent>(TEXT("FloatingDamageComponent"));
	FloatingDamageComponent->SetupAttachment(RootComponent);

	SelectionRingComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionRingComponent"));
	SelectionRingComponent->SetupAttachment(RootComponent);
	SelectionRingComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -94.0f));
	SelectionRingComponent->SetRelativeScale3D(FVector(2.45f, 2.45f, 0.018f));
	SelectionRingComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionRingComponent->SetHiddenInGame(true);
	SelectionRingComponent->SetCastShadow(false);

	SelectionLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("SelectionLightComponent"));
	SelectionLightComponent->SetupAttachment(RootComponent);
	SelectionLightComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 42.0f));
	SelectionLightComponent->SetLightColor(FLinearColor(1.0f, 0.62f, 0.08f, 1.0f));
	SelectionLightComponent->Intensity = 0.0f;
	SelectionLightComponent->AttenuationRadius = 440.0f;
	SelectionLightComponent->bUseInverseSquaredFalloff = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = PatrolSpeed;
		Movement->BrakingDecelerationWalking = 640.0f;
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.0f, 280.0f, 0.0f);
		Movement->SetComponentTickEnabled(true);
	}

	bUseControllerRotationYaw = false;
}

void ADBALobbyTrainingMonster::BeginPlay()
{
	Super::BeginPlay();
	InitialSpawnTransform = GetActorTransform();
	ApplyLobbyMonsterVisuals();
	UpdateHealthBar();
	ConfigurePatrolRoute();

	// P2-3 改造：注册 CurrentHealth 属性变化 Delegate，事件驱动血条 UI 更新。
	// 替代旧 OnRep_CurrentHealth 机制，符合《DBA.UI.EventAsync》事件驱动 UI 策略。
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (BattleAttributeSet != nullptr)
		{
			ASC->GetGameplayAttributeValueChangeDelegate(BattleAttributeSet->GetCurrentHealthAttribute())
				.AddUObject(this, &ADBALobbyTrainingMonster::HandleHealthAttributeChanged);
		}
	}
}

void ADBALobbyTrainingMonster::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateLobbyPatrol(DeltaSeconds);
	if (!HasAuthority())
	{
		RefreshPatrolAnimationFromVelocity();
	}
}

void ADBALobbyTrainingMonster::ConfigureLobbyMonster(int32 MonsterIndex)
{
	VisualIndex = MonsterIndex;
	ApplyLobbyMonsterVisuals();
	ConfigurePatrolRoute();
}

void ADBALobbyTrainingMonster::SetLobbySelected(bool bSelected)
{
	if (SelectionRingComponent)
	{
		SelectionRingComponent->SetHiddenInGame(!bSelected);
		SelectionRingComponent->SetVisibility(bSelected, true);
	}
	if (SelectionLightComponent)
	{
		SelectionLightComponent->Intensity = bSelected ? 5200.0f : 0.0f;
	}
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetRenderCustomDepth(bSelected);
		MeshComponent->SetCustomDepthStencilValue(bSelected ? 250 : 0);
	}
	if (HealthBarComponent)
	{
		HealthBarComponent->SetDrawSize(bSelected ? FVector2D(59.0f, 7.0f) : FVector2D(48.0f, 5.0f));
		if (UDBALobbyMonsterHealthBarWidget* HealthBar = Cast<UDBALobbyMonsterHealthBarWidget>(HealthBarComponent->GetUserWidgetObject()))
		{
			HealthBar->SetSelected(bSelected);
		}
	}
}

void ADBALobbyTrainingMonster::ApplyLobbyMonsterVisuals()
{
	// Dedicated Server 不渲染大厅怪物，跳过纯表现资源加载，避免服务器依赖客户端骨架、动画和材质链。
	if (IsRunningDedicatedServer())
	{
		if (USkeletalMeshComponent* MeshComponent = GetMesh())
		{
			MeshComponent->SetHiddenInGame(true);
			MeshComponent->SetComponentTickEnabled(false);
		}
		return;
	}

	RequestLobbyMonsterVisuals();
}

void ADBALobbyTrainingMonster::RequestLobbyMonsterVisuals()
{
	const UDBAZodiacVisualDeveloperSettings* VisualSettings = GetDefault<UDBAZodiacVisualDeveloperSettings>();
	if (!VisualSettings)
	{
		UE_LOG(LogDBACore, Error, TEXT("[DBALobbyTrainingMonster] 大厅怪物视觉配置不可用。"));
		return;
	}

	const TSoftObjectPtr<USkeletalMesh> MeshAsset = VisualSettings->LobbyTrainingMonsterMesh.IsNull()
		? VisualSettings->PlaceholderSkeletalMesh
		: VisualSettings->LobbyTrainingMonsterMesh;
	const TSoftObjectPtr<UMaterialInterface> TintMaterialAsset = VisualSettings->LobbyTrainingMonsterTintMaterial.IsNull()
		? VisualSettings->PlaceholderTintMaterial
		: VisualSettings->LobbyTrainingMonsterTintMaterial;

	if (MeshAsset.IsNull())
	{
		UE_LOG(LogDBACore, Error, TEXT("[DBALobbyTrainingMonster] 大厅怪物网格未配置，请在 DBA 生肖外观配置中设置。"));
		return;
	}

	const TWeakObjectPtr<ADBALobbyTrainingMonster> WeakThis(this);
	DBAAsyncAssetLoader::RequestAsyncAsset<USkeletalMesh>(this, MeshAsset, [WeakThis](USkeletalMesh* LoadedMesh)
	{
		if (ADBALobbyTrainingMonster* StrongThis = WeakThis.Get())
		{
			StrongThis->ApplyLoadedLobbyMonsterMesh(LoadedMesh);
		}
	});

	if (!TintMaterialAsset.IsNull())
	{
		DBAAsyncAssetLoader::RequestAsyncAsset<UMaterialInterface>(this, TintMaterialAsset, [WeakThis](UMaterialInterface* LoadedMaterial)
		{
			if (ADBALobbyTrainingMonster* StrongThis = WeakThis.Get())
			{
				StrongThis->LobbyTintMaterial = LoadedMaterial;
				StrongThis->ApplyLobbyMonsterTintMaterial();
				StrongThis->ApplyLobbyMonsterSelectionRing();
			}
		});
	}

	if (!VisualSettings->LobbyTrainingMonsterSelectionRingMesh.IsNull())
	{
		DBAAsyncAssetLoader::RequestAsyncAsset<UStaticMesh>(this, VisualSettings->LobbyTrainingMonsterSelectionRingMesh, [WeakThis](UStaticMesh* LoadedRingMesh)
		{
			if (ADBALobbyTrainingMonster* StrongThis = WeakThis.Get())
			{
				StrongThis->LobbySelectionRingMesh = LoadedRingMesh;
				StrongThis->ApplyLobbyMonsterSelectionRing();
			}
		});
	}

	const auto RequestLobbyAnimation = [this, WeakThis](const TSoftObjectPtr<UAnimationAsset>& AnimationAsset, const bool bIdleAnimation)
	{
		if (AnimationAsset.IsNull())
		{
			return;
		}

		DBAAsyncAssetLoader::RequestAsyncAsset<UAnimationAsset>(this, AnimationAsset, [WeakThis, bIdleAnimation](UAnimationAsset* LoadedAnimation)
		{
			if (ADBALobbyTrainingMonster* StrongThis = WeakThis.Get())
			{
				if (bIdleAnimation)
				{
					StrongThis->LobbyIdleAnimation = LoadedAnimation;
				}
				else
				{
					StrongThis->LobbyWalkAnimation = LoadedAnimation;
				}
				StrongThis->CurrentLobbyAnimation = nullptr;
				StrongThis->SetPatrolMovingAnimation(StrongThis->bReplicatedPatrolMoving || StrongThis->GetVelocity().SizeSquared2D() > FMath::Square(5.0f), false);
			}
		});
	};

	RequestLobbyAnimation(VisualSettings->LobbyTrainingMonsterIdleAnimation, true);
	RequestLobbyAnimation(VisualSettings->LobbyTrainingMonsterWalkAnimation, false);
}

void ADBALobbyTrainingMonster::ApplyLoadedLobbyMonsterMesh(USkeletalMesh* LoadedMesh)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent || !LoadedMesh)
	{
		return;
	}

	// 先清除蓝图默认状态残留，避免旧动画与新网格在切换瞬间发生骨架链接。
	MeshComponent->SetAnimInstanceClass(nullptr);
	MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	MeshComponent->SetAnimation(nullptr);
	MeshComponent->SetSkeletalMesh(LoadedMesh);
	// 大厅怪物与玩家模型均使用资源导入时的原始比例，避免实例配置改变视觉尺寸。
	MeshComponent->SetRelativeScale3D(FVector::OneVector);
	MeshComponent->SetVisibility(true);
	MeshComponent->SetHiddenInGame(false);
	MeshComponent->SetComponentTickEnabled(true);
	MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	CurrentLobbyAnimation = nullptr;
	ApplyLobbyMonsterTintMaterial();
	SetPatrolMovingAnimation(bReplicatedPatrolMoving || GetVelocity().SizeSquared2D() > FMath::Square(5.0f), false);

	UE_LOG(LogDBACore, Log, TEXT("[DBALobbyTrainingMonster] 大厅怪物网格已异步应用：怪物=%s 网格=%s 最大生命=%.1f"),
		*GetName(),
		*LoadedMesh->GetPathName(),
		GetMaxHealth());
}

void ADBALobbyTrainingMonster::ApplyLobbyMonsterTintMaterial()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent || !LobbyTintMaterial)
	{
		return;
	}

	const int32 TintIndex = FMath::Abs(VisualIndex) % UE_ARRAY_COUNT(LobbyMonsterTints);
	const FLinearColor Tint = LobbyMonsterTints[TintIndex];
	const int32 MaterialSlotCount = FMath::Max(1, MeshComponent->GetNumMaterials());
	for (int32 SlotIndex = 0; SlotIndex < MaterialSlotCount; ++SlotIndex)
	{
		if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(LobbyTintMaterial, this))
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Tint"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("BodyColor"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("PrimaryColor"), Tint);

			MeshComponent->SetMaterial(SlotIndex, DynamicMaterial);
		}
	}
}

void ADBALobbyTrainingMonster::ApplyLobbyMonsterSelectionRing()
{
	if (!SelectionRingComponent)
	{
		return;
	}

	if (LobbySelectionRingMesh)
	{
		SelectionRingComponent->SetStaticMesh(LobbySelectionRingMesh);
	}
	if (!LobbyTintMaterial)
	{
		return;
	}

	if (UMaterialInstanceDynamic* RingMaterial = UMaterialInstanceDynamic::Create(LobbyTintMaterial, this))
	{
		const FLinearColor RingColor(1.0f, 0.58f, 0.06f, 1.0f);
		RingMaterial->SetVectorParameterValue(TEXT("Tint"), RingColor);
		RingMaterial->SetVectorParameterValue(TEXT("BaseColor"), RingColor);
		RingMaterial->SetVectorParameterValue(TEXT("Color"), RingColor);
		RingMaterial->SetVectorParameterValue(TEXT("PrimaryColor"), RingColor);
		SelectionRingComponent->SetMaterial(0, RingMaterial);
	}
}

void ADBALobbyTrainingMonster::ConfigurePatrolRoute()
{
	if (!HasAuthority())
	{
		return;
	}

	PatrolPoints.Reset();
	const FVector Origin = GetActorLocation();
	const float PhaseDegrees = static_cast<float>(FMath::Abs(VisualIndex) % 8) * 22.5f;
	const float PhaseRadians = FMath::DegreesToRadians(PhaseDegrees);
	const FVector PatrolAxisA(FMath::Cos(PhaseRadians) * PatrolRadius, FMath::Sin(PhaseRadians) * PatrolRadius, 0.0f);
	const FVector PatrolAxisB(-PatrolAxisA.Y, PatrolAxisA.X, 0.0f);
	PatrolPoints.Add(Origin + PatrolAxisA);
	PatrolPoints.Add(Origin + PatrolAxisB);
	PatrolPoints.Add(Origin - PatrolAxisA);
	PatrolPoints.Add(Origin - PatrolAxisB);
	CurrentPatrolPointIndex = FMath::Abs(VisualIndex) % PatrolPoints.Num();
	NextPatrolMoveTime = GetWorld() ? GetWorld()->GetTimeSeconds() + 0.15f : 0.0f;
	bPatrolRouteConfigured = true;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
		Movement->MaxWalkSpeed = PatrolSpeed;
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBALobbyTrainingMonster] 巡逻路线已配置：怪物=%s 点数=%d 速度=%.1f 半径=%.1f"),
		*GetName(),
		PatrolPoints.Num(),
		PatrolSpeed,
		PatrolRadius);
}

void ADBALobbyTrainingMonster::UpdateLobbyPatrol(float DeltaSeconds)
{
	// P2-3 改造：使用 GetCurrentHealth() 读取 GAS AttributeSet 权威值，替代直接访问废弃字段。
	if (!HasAuthority() || bRespawning || !bPatrolRouteConfigured || PatrolPoints.Num() == 0 || GetCurrentHealth() <= 0.0f)
	{
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (Now < NextPatrolMoveTime)
	{
		SetPatrolMovingAnimation(false);
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->Velocity = FVector::ZeroVector;
		}
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = PatrolPoints[CurrentPatrolPointIndex];
	const FVector ToTarget = TargetLocation - CurrentLocation;
	const FVector FlatToTarget(ToTarget.X, ToTarget.Y, 0.0f);
	const float Distance = FlatToTarget.Size();
	if (Distance <= PatrolAcceptanceRadius)
	{
		AdvancePatrolTarget();
		NextPatrolMoveTime = Now + PatrolPauseSeconds;
		SetPatrolMovingAnimation(false);
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->Velocity = FVector::ZeroVector;
		}
		return;
	}

	const FVector Direction = FlatToTarget.GetSafeNormal();
	SetPatrolMovingAnimation(true);
	const FVector DeltaMove = Direction * PatrolSpeed * DeltaSeconds;
	FHitResult Hit;
	AddActorWorldOffset(DeltaMove, true, &Hit);
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->Velocity = Direction * PatrolSpeed;
	}
	SetActorRotation(Direction.Rotation());

	if (Hit.bBlockingHit)
	{
		AdvancePatrolTarget();
		NextPatrolMoveTime = Now + PatrolPauseSeconds;
	}
}

void ADBALobbyTrainingMonster::PlayLobbyMonsterAnimation(UAnimationAsset* AnimationAsset)
{
	if (!AnimationAsset || CurrentLobbyAnimation == AnimationAsset || !GetMesh())
	{
		return;
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	GetMesh()->SetAnimation(AnimationAsset);
	GetMesh()->Play(true);
	CurrentLobbyAnimation = AnimationAsset;
}

void ADBALobbyTrainingMonster::SetPatrolMovingAnimation(bool bMoving, bool bUpdateReplicatedState)
{
	if (bUpdateReplicatedState && HasAuthority())
	{
		bReplicatedPatrolMoving = bMoving;
	}

	PlayLobbyMonsterAnimation(bMoving && LobbyWalkAnimation ? LobbyWalkAnimation : LobbyIdleAnimation);
}

void ADBALobbyTrainingMonster::RefreshPatrolAnimationFromVelocity()
{
	const bool bMoving = bReplicatedPatrolMoving || GetVelocity().SizeSquared2D() > FMath::Square(5.0f);
	SetPatrolMovingAnimation(bMoving, false);
}

void ADBALobbyTrainingMonster::OnRep_ReplicatedPatrolMoving()
{
	SetPatrolMovingAnimation(bReplicatedPatrolMoving, false);
}

void ADBALobbyTrainingMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADBALobbyTrainingMonster, bReplicatedPatrolMoving);
}

void ADBALobbyTrainingMonster::AdvancePatrolTarget()
{
	CurrentPatrolPointIndex = PatrolPoints.Num() > 0
		? (CurrentPatrolPointIndex + 1) % PatrolPoints.Num()
		: 0;
}

void ADBALobbyTrainingMonster::HandleHealthAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	// P2-3 改造：CurrentHealth 属性变化时刷新大厅血条 UI（事件驱动，替代旧 OnRep_CurrentHealth）。
	UpdateHealthBar();
}

void ADBALobbyTrainingMonster::HandleMonsterDefeated(AActor* DamageCauser)
{
	if (bRespawning)
	{
		return;
	}

	bRespawning = true;
	PlayDeathVFX();
	SetActorEnableCollision(false);
	SetPatrolMovingAnimation(false);
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->Velocity = FVector::ZeroVector;
		Movement->DisableMovement();
	}
	MulticastSetDefeatedVisualState(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RespawnTimerHandle, this, &ADBALobbyTrainingMonster::RespawnAfterDefeat, RespawnDelaySeconds, false);
	}

	UE_LOG(LogDBACombat, Log, TEXT("[DBALobbyTrainingMonster] 训练假人被击败并等待复活：怪物=%s 来源=%s 延迟=%.2f"),
		*GetName(),
		*GetNameSafe(DamageCauser),
		RespawnDelaySeconds);
}

void ADBALobbyTrainingMonster::RespawnAfterDefeat()
{
	if (!HasAuthority())
	{
		return;
	}

	bRespawning = false;
	SetActorTransform(InitialSpawnTransform);

	// P2-3 改造：通过 AttributeSet 恢复 CurrentHealth 到 MaxHealth（权威路径走 GAS）。
	// 客户端血条 UI 由 ASC AttributeChange Delegate 事件驱动刷新，无需手动调用 OnRep_CurrentHealth。
	if (BattleAttributeSet != nullptr)
	{
		BattleAttributeSet->SetCurrentHealth(BattleAttributeSet->GetMaxHealth());
	}

	SetActorEnableCollision(true);
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
		Movement->MaxWalkSpeed = PatrolSpeed;
		Movement->Velocity = FVector::ZeroVector;
	}
	ConfigurePatrolRoute();
	MulticastSetDefeatedVisualState(false);

	UE_LOG(LogDBACombat, Log, TEXT("[DBALobbyTrainingMonster] 训练假人已复活：怪物=%s 生命=%.1f/%.1f"),
		*GetName(),
		GetCurrentHealth(),
		GetMaxHealth());
}

void ADBALobbyTrainingMonster::MulticastSetDefeatedVisualState_Implementation(bool bDefeated)
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetHiddenInGame(bDefeated);
		MeshComponent->SetVisibility(!bDefeated, true);
		if (bDefeated)
		{
			MeshComponent->SetRenderCustomDepth(false);
			MeshComponent->SetCustomDepthStencilValue(0);
		}
	}

	if (HealthBarComponent)
	{
		HealthBarComponent->SetHiddenInGame(bDefeated);
		HealthBarComponent->SetVisibility(!bDefeated, true);
	}

	if (SelectionRingComponent)
	{
		SelectionRingComponent->SetHiddenInGame(true);
	}
	if (SelectionLightComponent)
	{
		SelectionLightComponent->Intensity = 0.0f;
	}
}

void ADBALobbyTrainingMonster::UpdateHealthBar()
{
	if (!HealthBarComponent)
	{
		return;
	}

	if (UDBALobbyMonsterHealthBarWidget* HealthBar = Cast<UDBALobbyMonsterHealthBarWidget>(HealthBarComponent->GetUserWidgetObject()))
	{
		HealthBar->SetHealthPercent(GetHealthPercent());
	}
}
