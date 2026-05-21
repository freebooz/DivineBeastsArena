// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊妯″瀷鍩虹被

#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Combat/DBAFireballProjectile.h"
#include "GameDBA/Combat/DBAProjectile_Generic.h"
#include "GameDBA/Combat/DBASkillProjectileBase.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/RPC/DBARpcHandler.h"
#include "GameDBA/Services/DBASkillGroupGeneratorSubsystem.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameDBA/Animation/DBAZodiacAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"

namespace
{
	struct FLobbyEquippedSkillCastSpec
	{
		int32 SkillSlot = 1;
		FName FallbackSkillId = TEXT("Lobby.Skill01");
		float Damage = 35.0f;
		float Speed = 1450.0f;
		float Radius = 42.0f;
		float Cooldown = 3.0f;
		float CastVFXScale = 1.0f;
		const TCHAR* CastNiagaraPath = nullptr;
		const TCHAR* ProjectileNiagaraPath = nullptr;
		const TCHAR* ImpactNiagaraPath = nullptr;
		const TCHAR* CastSFXPath = nullptr;
		const TCHAR* FlySFXPath = nullptr;
		const TCHAR* ImpactSFXPath = nullptr;
	};

	const FLobbyEquippedSkillCastSpec& GetDefaultLobbySkillSpec(int32 SkillSlot)
	{
		static const FLobbyEquippedSkillCastSpec Skill01{
			1,
			TEXT("Lobby.Skill01.Fire"),
			35.0f,
			1450.0f,
			42.0f,
			3.0f,
			1.0f,
			TEXT("/Game/DBA/VFX/Abilities/FireLion/NS_FireLion_Q_FlameClaw_Slash.NS_FireLion_Q_FlameClaw_Slash"),
			TEXT("/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Projectile.NS_DBA_Fireball_Projectile"),
			TEXT("/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Impact.NS_DBA_Fireball_Impact"),
			TEXT("/Game/DBA/Audio/SFX/Abilities/FireLion/SFX_FireLion_Q_FlameClaw_Slash.SFX_FireLion_Q_FlameClaw_Slash"),
			TEXT("/Game/DBA/Audio/SFX/Common/Status/SFX_Status_Burning_Loop.SFX_Status_Burning_Loop"),
			TEXT("/Game/DBA/Audio/SFX/Abilities/FireLion/SFX_FireLion_Q_FlameClaw_Impact.SFX_FireLion_Q_FlameClaw_Impact")
		};
		static const FLobbyEquippedSkillCastSpec Skill02{
			2,
			TEXT("Lobby.Skill02.Water"),
			42.0f,
			1350.0f,
			46.0f,
			4.5f,
			1.0f,
			TEXT("/Game/DBA/VFX/Common/Status/NS_Status_Wet.NS_Status_Wet"),
			TEXT("/Game/DBA/VFX/Abilities/WaterDragon/NS_WaterDragon_Q_WaterBlast_Projectile.NS_WaterDragon_Q_WaterBlast_Projectile"),
			TEXT("/Game/DBA/VFX/Abilities/WaterDragon/NS_WaterDragon_Q_WaterBlast_Impact.NS_WaterDragon_Q_WaterBlast_Impact"),
			TEXT("/Game/DBA/Audio/SFX/Abilities/WaterDragon/SFX_WaterDragon_Q_WaterBlast_Cast.SFX_WaterDragon_Q_WaterBlast_Cast"),
			TEXT("/Game/DBA/Audio/SFX/Common/Status/SFX_Status_Wet.SFX_Status_Wet"),
			TEXT("/Game/DBA/Audio/SFX/Abilities/WaterDragon/SFX_WaterDragon_Q_WaterBlast_Impact.SFX_WaterDragon_Q_WaterBlast_Impact")
		};
		static const FLobbyEquippedSkillCastSpec Skill03{
			3,
			TEXT("Lobby.Skill03.Wood"),
			48.0f,
			1220.0f,
			48.0f,
			6.0f,
			1.05f,
			TEXT("/Game/DBA/VFX/Common/Status/NS_Status_HealingOverTime.NS_Status_HealingOverTime"),
			TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingSeed_Projectile.NS_WoodCrane_Q_HealingSeed_Projectile"),
			TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingBurst_Impact.NS_WoodCrane_Q_HealingBurst_Impact"),
			TEXT("/Game/DBA/Audio/SFX/Abilities/WoodCrane/SFX_WoodCrane_Q_HealingSeed_Cast.SFX_WoodCrane_Q_HealingSeed_Cast"),
			TEXT("/Game/DBA/Audio/SFX/Common/Status/SFX_Status_HealingOverTime.SFX_Status_HealingOverTime"),
			TEXT("/Game/DBA/Audio/SFX/Abilities/WoodCrane/SFX_WoodCrane_Q_HealingBurst_Impact.SFX_WoodCrane_Q_HealingBurst_Impact")
		};
		static const FLobbyEquippedSkillCastSpec Skill04{
			4,
			TEXT("Lobby.Skill04.Gold"),
			56.0f,
			1580.0f,
			38.0f,
			7.5f,
			1.0f,
			TEXT("/Game/DBA/VFX/Abilities/GoldPhoenix/NS_GoldPhoenix_Q_GoldFeather_Impact.NS_GoldPhoenix_Q_GoldFeather_Impact"),
			TEXT("/Game/DBA/VFX/Abilities/GoldPhoenix/NS_GoldPhoenix_Q_GoldFeather_Projectile.NS_GoldPhoenix_Q_GoldFeather_Projectile"),
			TEXT("/Game/DBA/VFX/Abilities/GoldPhoenix/NS_GoldPhoenix_Q_GoldFeather_Impact.NS_GoldPhoenix_Q_GoldFeather_Impact"),
			TEXT("/Game/DBA/Audio/SFX/Abilities/GoldPhoenix/SFX_GoldPhoenix_Q_GoldFeather_Cast.SFX_GoldPhoenix_Q_GoldFeather_Cast"),
			nullptr,
			TEXT("/Game/DBA/Audio/SFX/Abilities/GoldPhoenix/SFX_GoldPhoenix_Q_GoldFeather_Impact.SFX_GoldPhoenix_Q_GoldFeather_Impact")
		};
		static const FLobbyEquippedSkillCastSpec Ultimate{
			5,
			TEXT("Lobby.Ultimate"),
			86.0f,
			1700.0f,
			64.0f,
			12.0f,
			1.35f,
			TEXT("/Game/DBA/VFX/Abilities/FireLion/NS_FireLion_R_DivineBeastTransform.NS_FireLion_R_DivineBeastTransform"),
			TEXT("/Game/DBA/VFX/Abilities/FireLion/NS_FireLion_E_FlameLeap_Trail.NS_FireLion_E_FlameLeap_Trail"),
			TEXT("/Game/DBA/VFX/Common/Impact/NS_Impact_Magic_Burst.NS_Impact_Magic_Burst"),
			TEXT("/Game/DBA/Audio/SFX/Abilities/FireLion/SFX_FireLion_R_DivineBeastTransform.SFX_FireLion_R_DivineBeastTransform"),
			nullptr,
			TEXT("/Game/DBA/Audio/SFX/Abilities/FireLion/SFX_FireLion_E_FlameLeap_Impact.SFX_FireLion_E_FlameLeap_Impact")
		};

		switch (SkillSlot)
		{
		case 1: return Skill01;
		case 2: return Skill02;
		case 3: return Skill03;
		case 4: return Skill04;
		case 5: return Ultimate;
		default: return Skill01;
		}
	}

	bool IsLobbyEquippedSkillSlot(int32 SkillSlot)
	{
		return SkillSlot >= 1 && SkillSlot <= 5;
	}

	EDBAZodiac ToCommonZodiac(EDBAZodiacType ZodiacType)
	{
		switch (ZodiacType)
		{
		case EDBAZodiacType::Rat: return EDBAZodiac::Rat;
		case EDBAZodiacType::Ox: return EDBAZodiac::Ox;
		case EDBAZodiacType::Tiger: return EDBAZodiac::Tiger;
		case EDBAZodiacType::Rabbit: return EDBAZodiac::Rabbit;
		case EDBAZodiacType::Dragon: return EDBAZodiac::Dragon;
		case EDBAZodiacType::Snake: return EDBAZodiac::Snake;
		case EDBAZodiacType::Horse: return EDBAZodiac::Horse;
		case EDBAZodiacType::Goat: return EDBAZodiac::Goat;
		case EDBAZodiacType::Monkey: return EDBAZodiac::Monkey;
		case EDBAZodiacType::Rooster: return EDBAZodiac::Rooster;
		case EDBAZodiacType::Dog: return EDBAZodiac::Dog;
		case EDBAZodiacType::Pig: return EDBAZodiac::Pig;
		default: return EDBAZodiac::Rat;
		}
	}

	EDBAElement ToCommonElement(EDBAElementType ElementType)
	{
		switch (ElementType)
		{
		case EDBAElementType::Fire: return EDBAElement::Fire;
		case EDBAElementType::Water: return EDBAElement::Water;
		case EDBAElementType::Wood: return EDBAElement::Wood;
		case EDBAElementType::Metal: return EDBAElement::Gold;
		case EDBAElementType::Earth: return EDBAElement::Earth;
		default: return EDBAElement::Fire;
		}
	}

	FVector ResolveHorizontalAimDirection(const FVector& DesiredDirection, const FVector& FallbackDirection)
	{
		FVector HorizontalDirection(DesiredDirection.X, DesiredDirection.Y, 0.0f);
		if (HorizontalDirection.IsNearlyZero())
		{
			HorizontalDirection = FVector(FallbackDirection.X, FallbackDirection.Y, 0.0f);
		}
		if (HorizontalDirection.IsNearlyZero())
		{
			HorizontalDirection = FVector::ForwardVector;
		}
		return HorizontalDirection.GetSafeNormal();
	}

	FName ResolveEquippedLobbySkillId(const ADBAZodiacCharacterBase* Character, int32 SkillSlot)
	{
		const FLobbyEquippedSkillCastSpec& DefaultSpec = GetDefaultLobbySkillSpec(SkillSlot);
		if (!Character || !Character->GetWorld())
		{
			return DefaultSpec.FallbackSkillId;
		}

		UGameInstance* GameInstance = Character->GetWorld()->GetGameInstance();
		UDBASkillGroupGeneratorSubsystem* SkillGroups = GameInstance
			? GameInstance->GetSubsystem<UDBASkillGroupGeneratorSubsystem>()
			: nullptr;
		if (!SkillGroups)
		{
			return DefaultSpec.FallbackSkillId;
		}

		FDBAZodiacElementFixedSkillGroupRow SkillGroup;
		if (!SkillGroups->GetSkillGroup(ToCommonZodiac(Character->GetZodiacType()), ToCommonElement(Character->GetElementType()), SkillGroup))
		{
			return DefaultSpec.FallbackSkillId;
		}

		switch (SkillSlot)
		{
		case 1: return SkillGroup.ElementSkill1Id.IsNone() ? DefaultSpec.FallbackSkillId : SkillGroup.ElementSkill1Id;
		case 2: return SkillGroup.ElementSkill2Id.IsNone() ? DefaultSpec.FallbackSkillId : SkillGroup.ElementSkill2Id;
		case 3: return SkillGroup.ElementSkill3Id.IsNone() ? DefaultSpec.FallbackSkillId : SkillGroup.ElementSkill3Id;
		case 4: return SkillGroup.ElementSkill4Id.IsNone() ? DefaultSpec.FallbackSkillId : SkillGroup.ElementSkill4Id;
		case 5: return SkillGroup.ZodiacUltimateSkillId.IsNone() ? DefaultSpec.FallbackSkillId : SkillGroup.ZodiacUltimateSkillId;
		default: return DefaultSpec.FallbackSkillId;
		}
	}

	void SetSoftNiagaraAsset(TSoftObjectPtr<UNiagaraSystem>& OutAsset, const TCHAR* AssetPath)
	{
		if (AssetPath && FCString::Strlen(AssetPath) > 0)
		{
			OutAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(AssetPath));
		}
	}

	void SetSoftSoundAsset(TSoftObjectPtr<USoundBase>& OutAsset, const TCHAR* AssetPath)
	{
		if (AssetPath && FCString::Strlen(AssetPath) > 0)
		{
			OutAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(AssetPath));
		}
	}

	void ApplyLobbySkillProjectileAssets(ADBASkillProjectileBase* Projectile, const FLobbyEquippedSkillCastSpec& Spec)
	{
		if (!Projectile)
		{
			return;
		}

		SetSoftNiagaraAsset(Projectile->ProjectileNiagaraVFXAsset, Spec.ProjectileNiagaraPath);
		SetSoftNiagaraAsset(Projectile->ImpactNiagaraVFXAsset, Spec.ImpactNiagaraPath);
		SetSoftSoundAsset(Projectile->FlySFXAsset, Spec.FlySFXPath);
		SetSoftSoundAsset(Projectile->ImpactSFXAsset, Spec.ImpactSFXPath);
	}
}

ADBAZodiacCharacterBase::ADBAZodiacCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	// 閰嶇疆纰版挒
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	LobbyCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("LobbyCameraBoom"));
	LobbyCameraBoom->SetupAttachment(RootComponent);
	LobbyCameraBoom->TargetArmLength = 520.0f;
	LobbyCameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 72.0f));
	LobbyCameraBoom->SetRelativeRotation(FRotator(-18.0f, 0.0f, 0.0f));
	LobbyCameraBoom->bUsePawnControlRotation = true;
	LobbyCameraBoom->bInheritPitch = true;
	LobbyCameraBoom->bInheritYaw = true;
	LobbyCameraBoom->bInheritRoll = false;
	LobbyCameraBoom->bDoCollisionTest = true;

	LobbyFollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("LobbyFollowCamera"));
	LobbyFollowCamera->SetupAttachment(LobbyCameraBoom, USpringArmComponent::SocketName);
	LobbyFollowCamera->bUsePawnControlRotation = false;

	// 閰嶇疆绉诲姩閫熷害
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MaxWalkSpeed;
		Movement->MaxWalkSpeedCrouched = MaxWalkSpeed;
		Movement->BrakingDecelerationWalking = BrakingDeceleration;
		Movement->bOrientRotationToMovement = false;
		Movement->bUseControllerDesiredRotation = false;
		Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	}

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	LobbyFireballProjectileClass = ADBAFireballProjectile::StaticClass();
	SkillCooldowns.Init(0.0f, 7);
	SkillMaxCooldowns.Init(0.0f, 7);
	for (int32 SkillSlot = 1; SkillSlot <= 5; ++SkillSlot)
	{
		SkillMaxCooldowns[SkillSlot] = GetDefaultLobbySkillSpec(SkillSlot).Cooldown;
	}
}

void ADBAZodiacCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	ApplyLobbyVisuals();

	// Spawn RPC Handler
	if (HasAuthority() && RpcHandlerClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		RpcHandler = GetWorld()->SpawnActor<ADBARpcHandler>(RpcHandlerClass.Get(), FTransform(FRotator::ZeroRotator, GetActorLocation()), SpawnParams);
		if (RpcHandler)
		{
			RpcHandler->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
}

void ADBAZodiacCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		for (float& Cooldown : SkillCooldowns)
		{
			if (Cooldown > 0.0f)
			{
				Cooldown = FMath::Max(0.0f, Cooldown - DeltaSeconds);
			}
		}
		if (SkillMaxCooldowns.Num() < 7)
		{
			SkillMaxCooldowns.SetNumZeroed(7);
		}
		for (int32 SkillSlot = 1; SkillSlot <= 5; ++SkillSlot)
		{
			const float DefaultCooldown = SkillSlot == 1 ? LobbyFireballCooldown : GetDefaultLobbySkillSpec(SkillSlot).Cooldown;
			SkillMaxCooldowns[SkillSlot] = FMath::Max(SkillMaxCooldowns[SkillSlot], DefaultCooldown);
		}
	}

	if (LobbyAttackAnimationTimeRemaining > 0.0f)
	{
		LobbyAttackAnimationTimeRemaining = FMath::Max(0.0f, LobbyAttackAnimationTimeRemaining - DeltaSeconds);
	}

	UpdateLobbyLocomotionAnimation();
}

void ADBAZodiacCharacterBase::ApplyLobbyVisuals()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return;
	}

	const EDBAZodiac CommonZodiac = ToCommonZodiac(ZodiacType);
	const TArray<FString> MeshCandidates = ADBACharacterPresentationActor::GetLobbyDisplayMeshCandidatePathsForZodiac(CommonZodiac);

	USkeletalMesh* ResolvedMesh = nullptr;
	FString ResolvedMeshPath;
	for (const FString& MeshPath : MeshCandidates)
	{
		if (MeshPath.IsEmpty())
		{
			continue;
		}

		ResolvedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
		if (ResolvedMesh)
		{
			ResolvedMeshPath = MeshPath;
			if (ResolvedMeshPath.Contains(TEXT("/Game/DBA/Characters/Rosales/")))
			{
				if (USkeleton* RosalesSkeleton = LoadObject<USkeleton>(nullptr, TEXT("/Game/DBA/Characters/Rosales/Meshes/SKEL_Rosales.SKEL_Rosales")))
				{
					ResolvedMesh->SetSkeleton(RosalesSkeleton);
				}
			}
			break;
		}
	}

	if (ResolvedMesh)
	{
		MeshComponent->SetSkeletalMesh(ResolvedMesh);
		const float MeshScale = 1.0f;
		const float CapsuleHalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.0f;
		const FBox MeshBox = ResolvedMesh->GetBounds().GetBox();
		const float MeshBottomOffsetZ = MeshBox.IsValid ? (-MeshBox.Min.Z * MeshScale) + 2.0f : 0.0f;
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -CapsuleHalfHeight + MeshBottomOffsetZ));
		MeshComponent->SetRelativeRotation(ADBACharacterPresentationActor::GetPreviewMeshPlayerFacingRotation());
		MeshComponent->SetRelativeScale3D(FVector(MeshScale));

		const bool bUseRosalesSingleNodeAnimation = ResolvedMeshPath.Contains(TEXT("/Game/DBA/Characters/Rosales/"));
		if (ResolvedMesh->GetSkeleton())
		{
			if (bUseRosalesSingleNodeAnimation)
			{
				LobbyIdleAnimation = LoadLobbyAnimation(TEXT("/Game/DBA/Characters/Rosales/Animations/AN_Standing_Idle.AN_Standing_Idle"));
				LobbyRunAnimation = LoadLobbyAnimation(TEXT("/Game/DBA/Characters/Rosales/Animations/AN_Run_Forward.AN_Run_Forward"));
				LobbyAttackAnimation = LoadLobbyAnimation(TEXT("/Game/DBA/Characters/Rosales/Animations/AN_Standing_2H_Magic_Attack_02.AN_Standing_2H_Magic_Attack_02"));
				bUseLobbySingleNodeLocomotion = LobbyIdleAnimation || LobbyRunAnimation;
				CurrentLobbyAnimation = nullptr;
				UpdateLobbyLocomotionAnimation();
			}
			else if (!ADBACharacterPresentationActor::ApplyLobbyDisplayAnimationToMesh(MeshComponent, ResolvedMeshPath, CommonZodiac))
			{
				MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				MeshComponent->SetAnimInstanceClass(UDBAZodiacAnimInstance::StaticClass());
			}
		}
		else
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			UE_LOG(LogTemp, Warning, TEXT("[DBAZodiacCharacterBase] 网格没有骨骼，已跳过动画蓝图：%s"), *ResolvedMeshPath);
		}
	}

	MeshComponent->SetVisibility(true);
	MeshComponent->SetHiddenInGame(false);
	MeshComponent->SetComponentTickEnabled(true);
	MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	MeshComponent->SetBoundsScale(2.0f);
	MeshComponent->UpdateBounds();
	ADBACharacterPresentationActor::ApplyZodiacMaterialToMesh(MeshComponent, CommonZodiac, this);

	UE_LOG(LogTemp, Log, TEXT("[DBAZodiacCharacterBase] 大厅角色外观已应用：Actor=%s 生肖=%d 网格=%s 骨骼=%s 动画=%s 相对位置=%s 相对旋转=%s"),
		*GetName(),
		static_cast<int32>(CommonZodiac),
		ResolvedMeshPath.IsEmpty() ? TEXT("<unchanged>") : *ResolvedMeshPath,
		ResolvedMesh && ResolvedMesh->GetSkeleton() ? TEXT("是") : TEXT("否"),
		MeshComponent->GetAnimInstance() ? *MeshComponent->GetAnimInstance()->GetClass()->GetName() : *GetNameSafe(CurrentLobbyAnimation),
		*MeshComponent->GetRelativeLocation().ToString(),
		*MeshComponent->GetRelativeRotation().ToString());
}

void ADBAZodiacCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ADBAZodiacCharacterBase::CastLobbyFireball()
{
	CastEquippedSkill(1);
}

void ADBAZodiacCharacterBase::CastLobbyFireballAtTarget(AActor* TargetActor)
{
	CastEquippedSkillAtTarget(1, TargetActor);
}

void ADBAZodiacCharacterBase::CastEquippedSkill(int32 SkillSlot)
{
	FVector AimDirection = GetActorForwardVector();
	if (const AController* OwningController = GetController())
	{
		AimDirection = OwningController->GetControlRotation().Vector();
	}

	AimDirection = ResolveHorizontalAimDirection(AimDirection, GetActorForwardVector());
	if (AimDirection.IsNearlyZero())
	{
		AimDirection = GetActorForwardVector();
	}

	if (!HasAuthority())
	{
		ServerCastEquippedSkill(SkillSlot, nullptr, AimDirection);
		return;
	}

	CastEquippedSkillInternal(SkillSlot, AimDirection);
}

void ADBAZodiacCharacterBase::CastEquippedSkillAtTarget(int32 SkillSlot, AActor* TargetActor)
{
	FVector AimDirection = GetActorForwardVector();
	if (TargetActor)
	{
		AimDirection = ResolveHorizontalAimDirection(TargetActor->GetActorLocation() - GetActorLocation(), GetActorForwardVector());
	}
	else if (const AController* OwningController = GetController())
	{
		AimDirection = OwningController->GetControlRotation().Vector();
	}

	AimDirection = ResolveHorizontalAimDirection(AimDirection, GetActorForwardVector());
	if (AimDirection.IsNearlyZero())
	{
		AimDirection = GetActorForwardVector();
	}

	if (!HasAuthority())
	{
		ServerCastEquippedSkill(SkillSlot, TargetActor, AimDirection);
		return;
	}

	CastEquippedSkillInternal(SkillSlot, AimDirection, TargetActor);
}

void ADBAZodiacCharacterBase::ServerCastLobbyFireball_Implementation(FVector_NetQuantizeNormal AimDirection)
{
	CastEquippedSkillInternal(1, FVector(AimDirection));
}

void ADBAZodiacCharacterBase::ServerCastLobbyFireballAtTarget_Implementation(AActor* TargetActor, FVector_NetQuantizeNormal FallbackAimDirection)
{
	CastEquippedSkillInternal(1, FVector(FallbackAimDirection), TargetActor);
}

void ADBAZodiacCharacterBase::ServerCastEquippedSkill_Implementation(int32 SkillSlot, AActor* TargetActor, FVector_NetQuantizeNormal FallbackAimDirection)
{
	CastEquippedSkillInternal(SkillSlot, FVector(FallbackAimDirection), TargetActor);
}

void ADBAZodiacCharacterBase::CastLobbyFireballInternal(const FVector& AimDirection, AActor* TargetActor)
{
	CastEquippedSkillInternal(1, AimDirection, TargetActor);
}

void ADBAZodiacCharacterBase::CastEquippedSkillInternal(int32 SkillSlot, const FVector& AimDirection, AActor* TargetActor)
{
	if (!GetWorld())
	{
		return;
	}

	if (!IsLobbyEquippedSkillSlot(SkillSlot))
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 装配技能槽位无效：施法者=%s 槽位=%d"),
			*GetName(),
			SkillSlot);
		return;
	}

	FLobbyEquippedSkillCastSpec Spec = GetDefaultLobbySkillSpec(SkillSlot);
	Spec.FallbackSkillId = ResolveEquippedLobbySkillId(this, SkillSlot);
	if (SkillSlot == 1)
	{
		Spec.Damage = LobbyFireballDamage;
		Spec.Speed = LobbyFireballSpeed;
		Spec.Radius = LobbyFireballRadius;
		Spec.Cooldown = LobbyFireballCooldown;
	}

	if (SkillCooldowns.Num() < 7)
	{
		SkillCooldowns.SetNumZeroed(7);
	}
	if (SkillMaxCooldowns.Num() < 7)
	{
		SkillMaxCooldowns.SetNumZeroed(7);
	}
	SkillMaxCooldowns[SkillSlot] = FMath::Max(SkillMaxCooldowns[SkillSlot], Spec.Cooldown);
	if (SkillCooldowns.IsValidIndex(SkillSlot) && SkillCooldowns[SkillSlot] > 0.0f)
	{
		UE_LOG(LogDBACombat, Verbose, TEXT("[DBAZodiacCharacterBase] 装配技能被冷却阻止：施法者=%s 槽位=%d 技能=%s 剩余=%.2f"),
			*GetName(),
			SkillSlot,
			*Spec.FallbackSkillId.ToString(),
			SkillCooldowns[SkillSlot]);
		return;
	}

	FVector SafeAimDirection = ResolveHorizontalAimDirection(AimDirection, GetActorForwardVector());
	if (IsValid(TargetActor) && TargetActor != this)
	{
		SafeAimDirection = ResolveHorizontalAimDirection(TargetActor->GetActorLocation() - GetActorLocation(), SafeAimDirection);
	}

	TSubclassOf<ADBASkillProjectileBase> ProjectileClass = LoadClass<ADBASkillProjectileBase>(
		nullptr,
		TEXT("/Game/DBA/Blueprints/Projectiles/BP_DBA_FireballProjectile.BP_DBA_FireballProjectile_C"));
	if (SkillSlot != 1 || !ProjectileClass)
	{
		if (SkillSlot == 1)
		{
			ProjectileClass = LobbyFireballProjectileClass;
			if (!ProjectileClass)
			{
				ProjectileClass = ADBAFireballProjectile::StaticClass();
			}
		}
		else
		{
			ProjectileClass = ADBAProjectile_Generic::StaticClass();
		}
	}

	const FVector SpawnLocation = GetActorLocation() + SafeAimDirection * 110.0f + FVector(0.0f, 0.0f, 74.0f);
	const FRotator SpawnRotation = SafeAimDirection.Rotation();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADBASkillProjectileBase* Fireball = GetWorld()->SpawnActor<ADBASkillProjectileBase>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);
	if (!Fireball)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 生成装配技能投射物失败：槽位=%d 技能=%s 类=%s"),
			SkillSlot,
			*Spec.FallbackSkillId.ToString(),
			*GetNameSafe(ProjectileClass));
		return;
	}

	ApplyLobbySkillProjectileAssets(Fireball, Spec);
	Fireball->InitializeProjectile(Spec.FallbackSkillId, this, TargetActor, Spec.Damage, Spec.Speed, Spec.Radius);
	Fireball->LaunchProjectile(SafeAimDirection);
	SkillCooldowns[SkillSlot] = Spec.Cooldown;
	SkillMaxCooldowns[SkillSlot] = Spec.Cooldown;
	MulticastPlayLobbySkillCastFeedback(SkillSlot);

	UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacCharacterBase] 已施放装配技能：施法者=%s 槽位=%d 技能=%s 投射物=%s 类=%s 目标=%s 水平方向=%s"),
		*GetName(),
		SkillSlot,
		*Spec.FallbackSkillId.ToString(),
		*Fireball->GetName(),
		*GetNameSafe(ProjectileClass),
		*GetNameSafe(TargetActor),
		*SafeAimDirection.ToString());
}

void ADBAZodiacCharacterBase::MulticastPlayLobbySkillCastFeedback_Implementation(int32 SkillSlot)
{
	PlayLobbySkillCastFeedbackLocal(SkillSlot);
}

void ADBAZodiacCharacterBase::PlayLobbySkillCastFeedbackLocal(int32 SkillSlot)
{
	const FLobbyEquippedSkillCastSpec& Spec = GetDefaultLobbySkillSpec(SkillSlot);

	if (bUseLobbySingleNodeLocomotion && LobbyAttackAnimation)
	{
		LobbyAttackAnimationTimeRemaining = LobbyAttackAnimationDuration;
		CurrentLobbyAnimation = nullptr;
		UpdateLobbyLocomotionAnimation();
	}
	else
	{
		PlayAttackAnimation();
	}

	if (GetNetMode() == NM_DedicatedServer || !GetWorld())
	{
		return;
	}

	const FVector CastLocation = GetActorLocation() + GetActorForwardVector() * 72.0f + FVector(0.0f, 0.0f, 88.0f);
	if (Spec.CastNiagaraPath && FCString::Strlen(Spec.CastNiagaraPath) > 0)
	{
		if (UNiagaraSystem* CastVFX = LoadObject<UNiagaraSystem>(nullptr, Spec.CastNiagaraPath))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				CastVFX,
				CastLocation,
				GetActorRotation(),
				FVector(Spec.CastVFXScale),
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
		}
	}

	if (Spec.CastSFXPath && FCString::Strlen(Spec.CastSFXPath) > 0)
	{
		if (USoundBase* CastSFX = LoadObject<USoundBase>(nullptr, Spec.CastSFXPath))
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), CastSFX, CastLocation, 0.85f);
		}
	}
}

UDBAZodiacAnimInstance* ADBAZodiacCharacterBase::GetZodiacAnimInstance() const
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		return Cast<UDBAZodiacAnimInstance>(MeshComp->GetAnimInstance());
	}
	return nullptr;
}

UAnimationAsset* ADBAZodiacCharacterBase::LoadLobbyAnimation(const FString& AnimationPath)
{
	if (AnimationPath.IsEmpty())
	{
		return nullptr;
	}

	UAnimationAsset* Animation = LoadObject<UAnimationAsset>(nullptr, *AnimationPath);
	if (!Animation)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DBAZodiacCharacterBase] 加载大厅动画失败：%s"), *AnimationPath);
	}
	return Animation;
}

void ADBAZodiacCharacterBase::UpdateLobbyLocomotionAnimation()
{
	if (!bUseLobbySingleNodeLocomotion)
	{
		return;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return;
	}

	const bool bPlayingAttack = LobbyAttackAnimationTimeRemaining > 0.0f && LobbyAttackAnimation;
	UAnimationAsset* DesiredAnimation = nullptr;
	if (bPlayingAttack)
	{
		DesiredAnimation = LobbyAttackAnimation;
	}
	else
	{
		const float Speed2D = GetVelocity().Size2D();
		DesiredAnimation = (Speed2D > LobbyRunAnimationThreshold && LobbyRunAnimation)
			? LobbyRunAnimation
			: LobbyIdleAnimation;
	}

	if (!DesiredAnimation || DesiredAnimation == CurrentLobbyAnimation)
	{
		return;
	}

	MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	MeshComponent->SetAnimation(DesiredAnimation);
	MeshComponent->Play(!bPlayingAttack);
	CurrentLobbyAnimation = DesiredAnimation;

	UE_LOG(LogTemp, Log, TEXT("[DBAZodiacCharacterBase] 已应用大厅单节点动画：Actor=%s 动画=%s 循环=%s 速度=%s"),
		*GetName(),
		*DesiredAnimation->GetPathName(),
		bPlayingAttack ? TEXT("否") : TEXT("是"),
		*GetVelocity().ToString());
}

UDBAAbilitySystemComponent* ADBAZodiacCharacterBase::GetDBAAbilitySystemComponent() const
{
	// 浠庢嫢鏈夎€呰幏鍙朅bilitySystemComponent
	if (AActor* OwnerActor = GetOwner())
	{
		return Cast<UDBAAbilitySystemComponent>(OwnerActor->FindComponentByClass<UDBAAbilitySystemComponent>());
	}
	return nullptr;
}

// ==================== 灞炴€ц闂疄鐜?====================

float ADBAZodiacCharacterBase::GetCurrentHealth() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetNumericAttributeBase(UDBABattleAttributeSet::GetCurrentHealthAttribute());
	}
	return 0.0f;
}

float ADBAZodiacCharacterBase::GetMaxHealth() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetNumericAttributeBase(UDBABattleAttributeSet::GetMaxHealthAttribute());
	}
	return 0.0f;
}

float ADBAZodiacCharacterBase::GetCurrentEnergy() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetNumericAttributeBase(UDBABattleAttributeSet::GetCurrentEnergyAttribute());
	}
	return 0.0f;
}

void ADBAZodiacCharacterBase::SetUltimateEnergy(float Value)
{
	if (HasAuthority())
	{
		UltimateEnergy = FMath::Clamp(Value, 0.0f, 100.0f);
	}
}

void ADBAZodiacCharacterBase::AddUltimateEnergy(float Delta)
{
	if (HasAuthority())
	{
		UltimateEnergy = FMath::Clamp(UltimateEnergy + Delta, 0.0f, 100.0f);
	}
}

void ADBAZodiacCharacterBase::AddChainLevel(int32 Delta)
{
	if (HasAuthority())
	{
		ChainLevel = FMath::Clamp(ChainLevel + Delta, 0, 10);
	}
}

void ADBAZodiacCharacterBase::ResetChainLevel()
{
	if (HasAuthority())
	{
		ChainLevel = 0;
	}
}
void ADBAZodiacCharacterBase::PlayAttackAnimation()
{
	if (UDBAZodiacAnimInstance* Anim = GetZodiacAnimInstance())
	{
		Anim->SetIsAttacking(true);
	}
}

void ADBAZodiacCharacterBase::PlayHitAnimation()
{
	if (UDBAZodiacAnimInstance* Anim = GetZodiacAnimInstance())
	{
		Anim->SetIsHit(true);
	}
}

void ADBAZodiacCharacterBase::PlayDeathAnimation()
{
	if (UDBAZodiacAnimInstance* Anim = GetZodiacAnimInstance())
	{
		Anim->SetIsDead(true);
	}
}

void ADBAZodiacCharacterBase::SetAnimMoveSpeed(float Speed)
{
	if (UDBAZodiacAnimInstance* AnimInst = GetZodiacAnimInstance())
	{
		AnimInst->SetMoveSpeed(Speed);
	}
}

// ==================== 姝讳骸鐘舵€佸疄鐜?====================

void ADBAZodiacCharacterBase::OnDeath()
{
	if (HasAuthority())
	{
		DeathState = EDADeathState::Dying;
		PlayDeathAnimation();

		// 寤惰繜璁剧疆涓篋ead鐘舵€侊紝绛夊緟姝讳骸鍔ㄧ敾鎾斁
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			DeathState = EDADeathState::Dead;
		});
	}
}

void ADBAZodiacCharacterBase::OnRevive()
{
	if (HasAuthority())
	{
		DeathState = EDADeathState::Alive;
		// Revive health is controlled by the caller.
	}
}

void ADBAZodiacCharacterBase::UpdateSkillCooldowns(const TArray<float>& NewCooldowns)
{
	if (HasAuthority())
	{
		SkillCooldowns = NewCooldowns;

		// 鏇存柊鏈€澶у喎鍗存暟缁?(濡傛灉姣斿綋鍓嶈褰曠殑鏇村ぇ)
		for (int32 i = 0; i < NewCooldowns.Num(); ++i)
		{
			if (i >= SkillMaxCooldowns.Num())
			{
				SkillMaxCooldowns.Add(NewCooldowns[i]);
			}
			else if (NewCooldowns[i] > SkillMaxCooldowns[i])
			{
				SkillMaxCooldowns[i] = NewCooldowns[i];
			}
		}
	}
}

void ADBAZodiacCharacterBase::GetSpectatorData(FDBAObserverViewTarget& OutData) const
{
	OutData.TargetCharacter = MakeWeakObjectPtr(const_cast<ADBAZodiacCharacterBase*>(this));
	OutData.PlayerName = GetFName();
	OutData.TeamID = TeamID;
	OutData.HeroID = HeroID;
	OutData.CurrentHP = GetCurrentHealth();
	OutData.MaxHP = GetMaxHealth();
	OutData.CurrentEnergy = GetCurrentEnergy();
	OutData.MaxEnergy = 100.0f; // Set ultimate energy data.
	OutData.UltimateEnergy = UltimateEnergy;
	OutData.bUltimateReady = IsUltimateReady();
	OutData.SkillCooldowns = SkillCooldowns;
	OutData.SkillMaxCooldowns = SkillMaxCooldowns;
}

void ADBAZodiacCharacterBase::SetTeamID(int32 NewTeamID)
{
	if (HasAuthority())
	{
		TeamID = NewTeamID;
	}
}

bool ADBAZodiacCharacterBase::IsTeammate(const ADBAZodiacCharacterBase* Other) const
{
	if (!Other)
	{
		return false;
	}
	return TeamID == Other->TeamID;
}

// ==================== IIDBACharacterRef 接口实现 ====================

UAbilitySystemComponent* ADBAZodiacCharacterBase::GetAbilitySystemComponent() const
{
	return GetDBAAbilitySystemComponent();
}

EDBAElementType ADBAZodiacCharacterBase::GetElementType() const
{
	return ElementType;
}

float ADBAZodiacCharacterBase::GetMaxEnergy() const
{
	// 最大能量暂定为 100，后续可通过属性扩展
	return 100.0f;
}

bool ADBAZodiacCharacterBase::IsAbilityOnCooldown(FName SkillId) const
{
	// 简化实现：检查技能冷却数组
	// 实际实现应该通过 GAS AbilitySystemComponent 查询冷却
	return false;
}

bool ADBAZodiacCharacterBase::HasEnoughEnergy(float Cost) const
{
	return GetCurrentEnergy() >= Cost;
}




void ADBAZodiacCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADBAZodiacCharacterBase, UltimateEnergy);
	DOREPLIFETIME(ADBAZodiacCharacterBase, ChainLevel);
	DOREPLIFETIME(ADBAZodiacCharacterBase, ResonanceLevel);
	DOREPLIFETIME(ADBAZodiacCharacterBase, DeathState);
	DOREPLIFETIME(ADBAZodiacCharacterBase, TeamID);
	DOREPLIFETIME(ADBAZodiacCharacterBase, HeroID);
	DOREPLIFETIME(ADBAZodiacCharacterBase, SkillCooldowns);
	DOREPLIFETIME(ADBAZodiacCharacterBase, SkillMaxCooldowns);
}
