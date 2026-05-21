// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Character/Monster/DBALobbyTrainingMonster.h"

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
#include "GameDBA/Combat/Feedback/DBALobbyFloatingDamageComponent.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/Lobby/DBALobbyMonsterHealthBarWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"

namespace
{
	const TCHAR* const LobbyMonsterMeshPath = TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP.TutorialTPP");
	const TCHAR* const LobbyMonsterAnimBlueprintPath = TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP_AnimBlueprint.TutorialTPP_AnimBlueprint_C");
	const TCHAR* const LobbyMonsterIdleAnimationPath = TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/Tutorial_Idle.Tutorial_Idle");
	const TCHAR* const LobbyMonsterWalkAnimationPath = TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/Tutorial_Walk_Fwd.Tutorial_Walk_Fwd");
	const TCHAR* const LobbyMonsterFallbackMaterialPath = TEXT("/Game/DBA/Materials/M_DBA_RuntimeTint.M_DBA_RuntimeTint");
	const TCHAR* const LobbyMonsterBodyTexturePath = TEXT("/Game/DBA/Characters/Mannequins/Textures/Manny/T_Manny_01_D.T_Manny_01_D");
	const TCHAR* const LobbyMonsterAccentTexturePath = TEXT("/Game/DBA/Characters/Mannequins/Textures/Manny/T_Manny_02_D.T_Manny_02_D");

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
	MaxHealth = 120.0f;
	CurrentHealth = MaxHealth;
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
	SelectionRingComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	SelectionRingComponent->SetRelativeScale3D(FVector(1.25f, 1.25f, 0.025f));
	SelectionRingComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionRingComponent->SetHiddenInGame(true);

	SelectionLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("SelectionLightComponent"));
	SelectionLightComponent->SetupAttachment(RootComponent);
	SelectionLightComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 20.0f));
	SelectionLightComponent->SetLightColor(FLinearColor(1.0f, 0.16f, 0.04f, 1.0f));
	SelectionLightComponent->Intensity = 0.0f;
	SelectionLightComponent->AttenuationRadius = 300.0f;
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
	ApplyLobbyMonsterVisuals();
	UpdateHealthBar();
	ConfigurePatrolRoute();
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
		SelectionLightComponent->Intensity = bSelected ? 3200.0f : 0.0f;
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
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return;
	}

	const FString MeshPath(LobbyMonsterMeshPath);
	if (USkeletalMesh* LoadedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath))
	{
		MeshComponent->SetSkeletalMesh(LoadedMesh);
	}
	else
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBALobbyTrainingMonster] 加载大厅怪物网格失败：%s"), *MeshPath);
		return;
	}

	MeshComponent->SetRelativeScale3D(FVector(MeshScale));
	MeshComponent->SetVisibility(true);
	MeshComponent->SetHiddenInGame(false);
	MeshComponent->SetComponentTickEnabled(true);
	MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	const int32 TintIndex = FMath::Abs(VisualIndex) % UE_ARRAY_COUNT(LobbyMonsterTints);
	const FLinearColor Tint = LobbyMonsterTints[TintIndex];
	UMaterialInterface* ReliableMaterial = LoadObject<UMaterialInterface>(nullptr, LobbyMonsterFallbackMaterialPath);
	const int32 MaterialSlotCount = FMath::Max(1, MeshComponent->GetNumMaterials());
	for (int32 SlotIndex = 0; SlotIndex < MaterialSlotCount; ++SlotIndex)
	{
		UMaterialInterface* BaseMaterial = ReliableMaterial ? ReliableMaterial : MeshComponent->GetMaterial(SlotIndex);
		if (!BaseMaterial)
		{
			continue;
		}

		if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Tint"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("BodyColor"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("PrimaryColor"), Tint);

			const TCHAR* TexturePath = SlotIndex == 0 ? LobbyMonsterBodyTexturePath : LobbyMonsterAccentTexturePath;
			if (UTexture* AlbedoTexture = LoadObject<UTexture>(nullptr, TexturePath))
			{
				DynamicMaterial->SetTextureParameterValue(TEXT("AlbedoTexture"), AlbedoTexture);
				DynamicMaterial->SetTextureParameterValue(TEXT("BaseTexture"), AlbedoTexture);
				DynamicMaterial->SetTextureParameterValue(TEXT("DiffuseTexture"), AlbedoTexture);
			}

			MeshComponent->SetMaterial(SlotIndex, DynamicMaterial);
		}
	}

	LobbyIdleAnimation = LoadObject<UAnimationAsset>(nullptr, LobbyMonsterIdleAnimationPath);
	LobbyWalkAnimation = LoadObject<UAnimationAsset>(nullptr, LobbyMonsterWalkAnimationPath);
	MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	CurrentLobbyAnimation = nullptr;
	SetPatrolMovingAnimation(bReplicatedPatrolMoving || GetVelocity().SizeSquared2D() > FMath::Square(5.0f), false);
	if (!LobbyIdleAnimation || !LobbyWalkAnimation)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBALobbyTrainingMonster] 加载大厅怪物移动动画失败：待机=%s 行走=%s"),
			LobbyMonsterIdleAnimationPath,
			LobbyMonsterWalkAnimationPath);
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBALobbyTrainingMonster] 大厅怪物外观已应用：怪物=%s 网格=%s 材质=%s 材质槽=%d 颜色=%s 动画模式=单节点 待机=%s 行走=%s 生命=%.1f"),
		*GetName(),
		*MeshPath,
		ReliableMaterial ? *ReliableMaterial->GetPathName() : TEXT("<mesh-material>"),
		MaterialSlotCount,
		*Tint.ToString(),
		LobbyMonsterIdleAnimationPath,
		LobbyMonsterWalkAnimationPath,
		MaxHealth);

	if (UStaticMesh* RingMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		SelectionRingComponent->SetStaticMesh(RingMesh);
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
	const float Phase = static_cast<float>(FMath::Abs(VisualIndex) % 5) * 55.0f;
	PatrolPoints.Add(Origin + FVector(PatrolRadius, Phase, 0.0f));
	PatrolPoints.Add(Origin + FVector(Phase * 0.35f, PatrolRadius, 0.0f));
	PatrolPoints.Add(Origin + FVector(-PatrolRadius, -Phase, 0.0f));
	PatrolPoints.Add(Origin + FVector(-Phase * 0.35f, -PatrolRadius, 0.0f));
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
	if (!HasAuthority() || !bPatrolRouteConfigured || PatrolPoints.Num() == 0 || CurrentHealth <= 0.0f)
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

void ADBALobbyTrainingMonster::OnRep_CurrentHealth()
{
	Super::OnRep_CurrentHealth();
	UpdateHealthBar();
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
