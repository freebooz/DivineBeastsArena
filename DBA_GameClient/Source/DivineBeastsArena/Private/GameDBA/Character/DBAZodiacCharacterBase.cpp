// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 鐢熻倴瑙掕壊妯″瀷鍩虹被

#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Combat/DBABloomHealingSpell.h"
#include "GameDBA/Combat/DBAChainLightningSpell.h"
#include "GameDBA/Combat/DBAFireballProjectile.h"
#include "GameDBA/Combat/DBAFrostShardProjectile.h"
#include "GameDBA/Combat/DBAHolyShieldSpell.h"
#include "GameDBA/Combat/DBAPlayableSkillComponent.h"
#include "GameDBA/Combat/DBAProjectile_Generic.h"
#include "GameDBA/Combat/DBAShadowBoltProjectile.h"
#include "GameDBA/Combat/DBASkillProjectileBase.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/RPC/DBARpcHandler.h"
#include "GameDBA/Services/DBASkillGroupGeneratorSubsystem.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"
#include "Camera/CameraComponent.h"
#include "GameplayTagContainer.h"
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
		EDBAElement DamageElement = EDBAElement::Fire;
		float Speed = 1450.0f;
		float Radius = 42.0f;
		float Cooldown = 3.0f;
		float CastVFXScale = 1.0f;
		const TCHAR* ProjectileCueTagName = nullptr;
		const TCHAR* ImpactCueTagName = nullptr;
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
			TEXT("Lobby.Skill01.MageFireball"),
			42.0f,
			EDBAElement::Fire,
			1580.0f,
			46.0f,
			3.0f,
			1.12f,
			TEXT("GameplayCue.DBA.Skill.Projectile"),
			TEXT("GameplayCue.DBA.Skill.Impact"),
			TEXT("/Game/DBA/VFX/Abilities/FireLion/NS_FireLion_Q_FlameClaw_Slash.NS_FireLion_Q_FlameClaw_Slash"),
			TEXT("/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Projectile.NS_DBA_Fireball_Projectile"),
			TEXT("/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Impact.NS_DBA_Fireball_Impact"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_MageFireball_PreCast.SFX_MageFireball_PreCast"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_MageFireball_Flight.SFX_MageFireball_Flight"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_MageFireball_Impact.SFX_MageFireball_Impact")
		};
		static const FLobbyEquippedSkillCastSpec Skill02{
			2,
			TEXT("Lobby.Skill02.FrostShard"),
			32.0f,
			EDBAElement::Water,
			1840.0f,
			38.0f,
			4.5f,
			1.15f,
			TEXT("GameplayCue.DBA.Skill.Projectile"),
			TEXT("GameplayCue.DBA.Skill.Impact"),
			TEXT("/Game/ProjectileHitVFX/NS/NS_IceCrystal.NS_IceCrystal"),
			TEXT("/Game/ProjectileHitVFX/NS/NS_IceDart.NS_IceDart"),
			TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Ice_01.NS_Hit_Ice_01"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_FrostShard_PreCast.SFX_FrostShard_PreCast"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_FrostShard_Flight.SFX_FrostShard_Flight"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_FrostShard_Impact.SFX_FrostShard_Impact")
		};
		static const FLobbyEquippedSkillCastSpec Skill03{
			3,
			TEXT("Lobby.Skill03.BloomHealing"),
			115.0f,
			EDBAElement::Wood,
			0.0f,
			0.0f,
			5.5f,
			1.2f,
			TEXT("GameplayCue.DBA.Skill.Projectile"),
			TEXT("GameplayCue.DBA.Skill.Impact"),
			TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingGrove_Area.NS_WoodCrane_Q_HealingGrove_Area"),
			TEXT("/Game/DBA/VFX/Common/Impact/NS_Impact_Heal_Burst.NS_Impact_Heal_Burst"),
			TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingBurst_Impact.NS_WoodCrane_Q_HealingBurst_Impact"),
			nullptr,
			TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_Flight.SFX_BloomHealing_Flight"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_Impact.SFX_BloomHealing_Impact")
		};
		static const FLobbyEquippedSkillCastSpec Skill04{
			4,
			TEXT("Lobby.Skill04.ChainLightning"),
			38.0f,
			EDBAElement::Gold,
			0.0f,
			0.0f,
			6.0f,
			1.2f,
			TEXT("GameplayCue.DBA.Skill.Projectile"),
			TEXT("GameplayCue.DBA.Skill.Impact"),
			TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Eletric_01.NS_Hit_Eletric_01"),
			TEXT("/Game/ProjectileHitVFX/NS/NS_ThunderBolt.NS_ThunderBolt"),
			TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Thunder.NS_Hit_Thunder"),
			nullptr,
			TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_ChainLightning_Flight.SFX_ChainLightning_Flight"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_ChainLightning_Impact.SFX_ChainLightning_Impact")
		};
		static const FLobbyEquippedSkillCastSpec Ultimate{
			5,
			TEXT("Lobby.Skill05.PriestShield"),
			0.0f,
			EDBAElement::Wood,
			0.0f,
			0.0f,
			8.0f,
			1.22f,
			TEXT("GameplayCue.DBA.Skill.Projectile"),
			TEXT("GameplayCue.DBA.Skill.Impact"),
			nullptr,
			TEXT("/Game/DBA/VFX/Common/Status/NS_Status_Shielded.NS_Status_Shielded"),
			TEXT("/Game/ProjectileHitVFX/NS/NS_HolyEnergy.NS_HolyEnergy"),
			nullptr,
			TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_Flight.SFX_PriestShield_Flight"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_Impact.SFX_PriestShield_Impact")
		};
		static const FLobbyEquippedSkillCastSpec Skill06{
			6,
			TEXT("Lobby.Skill06.ShadowBolt"),
			44.0f,
			EDBAElement::Gold,
			1580.0f,
			40.0f,
			4.8f,
			1.10f,
			TEXT("GameplayCue.DBA.Skill.Projectile"),
			TEXT("GameplayCue.DBA.Skill.Impact"),
			TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Magic.NS_Hit_Magic"),
			TEXT("/Game/ProjectileHitVFX/NS/NS_PoisonSkullFish.NS_PoisonSkullFish"),
			TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Poison.NS_Hit_Poison"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_ShadowBolt_PreCast.SFX_ShadowBolt_PreCast"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_ShadowBolt_Flight.SFX_ShadowBolt_Flight"),
			TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_ShadowBolt_Impact.SFX_ShadowBolt_Impact")
		};

		switch (SkillSlot)
		{
		case 1: return Skill01;
		case 2: return Skill02;
		case 3: return Skill03;
		case 4: return Skill04;
		case 5: return Ultimate;
		case 6: return Skill06;
		default: return Skill01;
		}
	}

	bool IsLobbyEquippedSkillSlot(int32 SkillSlot)
	{
		return SkillSlot >= 1 && SkillSlot <= 6;
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

	FGameplayTag ResolveOptionalGameplayCueTag(const TCHAR* TagName)
	{
		if (!TagName || FCString::Strlen(TagName) <= 0)
		{
			return FGameplayTag();
		}

		return FGameplayTag::RequestGameplayTag(FName(TagName), false);
	}

	bool ResolvePlayableSkillSpec(const ADBAZodiacCharacterBase* Character, int32 SkillSlot, FDBAPlayableSkillRuntimeSpec& OutSpec)
	{
		if (const UDBAPlayableSkillComponent* SkillComponent = Character ? Character->GetPlayableSkillComponent() : nullptr)
		{
			return SkillComponent->GetSkillSpec(SkillSlot, OutSpec);
		}
		return false;
	}

	void ApplyLobbySkillProjectileAssets(ADBASkillProjectileBase* Projectile, const FDBAPlayableSkillRuntimeSpec& Spec)
	{
		if (!Projectile)
		{
			return;
		}

		Projectile->DamageElement = Spec.Element;
		Projectile->ProjectileCueTag = Spec.ProjectileCueTag;
		Projectile->ImpactCueTag = Spec.ImpactCueTag;
		Projectile->ProjectileNiagaraVFXAsset = Spec.ProjectileNiagaraVFXAsset;
		Projectile->ImpactNiagaraVFXAsset = Spec.ImpactNiagaraVFXAsset;
		Projectile->FlySFXAsset = Spec.FlySFXAsset;
		Projectile->ImpactSFXAsset = Spec.ImpactSFXAsset;
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

	PlayableSkillComponent = CreateDefaultSubobject<UDBAPlayableSkillComponent>(TEXT("PlayableSkillComponent"));

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
	LobbyFrostShardProjectileClass = ADBAFrostShardProjectile::StaticClass();
	LobbyBloomHealingSpellClass = ADBABloomHealingSpell::StaticClass();
	LobbyChainLightningSpellClass = ADBAChainLightningSpell::StaticClass();
	LobbyHolyShieldSpellClass = ADBAHolyShieldSpell::StaticClass();
	LobbyShadowBoltProjectileClass = ADBAShadowBoltProjectile::StaticClass();
	SkillCooldowns.Init(0.0f, 7);
	SkillMaxCooldowns.Init(0.0f, 7);
	for (int32 SkillSlot = 1; SkillSlot <= 5; ++SkillSlot)
	{
		FDBAPlayableSkillRuntimeSpec SkillSpec;
		SkillMaxCooldowns[SkillSlot] = ResolvePlayableSkillSpec(this, SkillSlot, SkillSpec)
			? SkillSpec.Cooldown
			: GetDefaultLobbySkillSpec(SkillSlot).Cooldown;
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
			FDBAPlayableSkillRuntimeSpec SkillSpec;
			const float DefaultCooldown = ResolvePlayableSkillSpec(this, SkillSlot, SkillSpec)
				? SkillSpec.Cooldown
				: (SkillSlot == 1 ? LobbyFireballCooldown : GetDefaultLobbySkillSpec(SkillSlot).Cooldown);
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
		const float MeshScale = 4.0f / 3.0f;
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

	FDBAPlayableSkillRuntimeSpec Spec;
	if (!ResolvePlayableSkillSpec(this, SkillSlot, Spec))
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 未找到装配技能规格：施法者=%s 槽位=%d"),
			*GetName(),
			SkillSlot);
		return;
	}

	if (SkillSlot == 1)
	{
		Spec.Magnitude = LobbyFireballDamage;
		Spec.ProjectileSpeed = LobbyFireballSpeed;
		Spec.ProjectileRadius = LobbyFireballRadius;
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
			*Spec.SkillId.ToString(),
			SkillCooldowns[SkillSlot]);
		return;
	}

	FVector SafeAimDirection = ResolveHorizontalAimDirection(AimDirection, GetActorForwardVector());
	if (IsValid(TargetActor) && TargetActor != this)
	{
		SafeAimDirection = ResolveHorizontalAimDirection(TargetActor->GetActorLocation() - GetActorLocation(), SafeAimDirection);
	}

	const FVector SpawnLocation = GetActorLocation() + SafeAimDirection * 110.0f + FVector(0.0f, 0.0f, 74.0f);
	const FRotator SpawnRotation = SafeAimDirection.Rotation();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (Spec.EffectShape == EDBAPlayableSkillEffectShape::BloomHealing)
	{
		TSubclassOf<ADBABloomHealingSpell> BloomClass = Spec.BloomHealingClass ? Spec.BloomHealingClass : LobbyBloomHealingSpellClass;
		if (!BloomClass)
		{
			BloomClass = ADBABloomHealingSpell::StaticClass();
		}

		ADBABloomHealingSpell* BloomSpell = GetWorld()->SpawnActor<ADBABloomHealingSpell>(
			BloomClass,
			GetActorLocation(),
			GetActorRotation(),
			SpawnParams);
		if (!BloomSpell)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 生成绽放治疗法术失败：施法者=%s 技能=%s 类=%s"),
				*GetName(),
				*Spec.SkillId.ToString(),
				*GetNameSafe(BloomClass));
			return;
		}

		AActor* HealTarget = Cast<ADBAZodiacCharacterBase>(TargetActor) ? TargetActor : this;
		BloomSpell->CastBloomHealing(this, HealTarget);
		SkillCooldowns[SkillSlot] = Spec.Cooldown;
		SkillMaxCooldowns[SkillSlot] = Spec.Cooldown;
		MulticastPlayLobbySkillCastFeedback(SkillSlot);
		UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacCharacterBase] 已施放绽放治疗：施法者=%s 技能=%s 目标=%s 法术=%s"),
			*GetName(),
			*Spec.SkillId.ToString(),
			*GetNameSafe(HealTarget),
			*BloomSpell->GetName());
		return;
	}

	if (Spec.EffectShape == EDBAPlayableSkillEffectShape::ChainLightning)
	{
		TSubclassOf<ADBAChainLightningSpell> ChainClass = Spec.ChainLightningClass ? Spec.ChainLightningClass : LobbyChainLightningSpellClass;
		if (!ChainClass)
		{
			ChainClass = ADBAChainLightningSpell::StaticClass();
		}

		ADBAChainLightningSpell* ChainSpell = GetWorld()->SpawnActor<ADBAChainLightningSpell>(
			ChainClass,
			GetActorLocation(),
			GetActorRotation(),
			SpawnParams);
		if (!ChainSpell)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 生成链式闪电法术失败：施法者=%s 技能=%s 类=%s"),
				*GetName(),
				*Spec.SkillId.ToString(),
				*GetNameSafe(ChainClass));
			return;
		}

		ChainSpell->CastChainLightning(this, TargetActor);
		SkillCooldowns[SkillSlot] = Spec.Cooldown;
		SkillMaxCooldowns[SkillSlot] = Spec.Cooldown;
		MulticastPlayLobbySkillCastFeedback(SkillSlot);
		UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacCharacterBase] 已施放链式闪电：施法者=%s 技能=%s 初始目标=%s 法术=%s"),
			*GetName(),
			*Spec.SkillId.ToString(),
			*GetNameSafe(TargetActor),
			*ChainSpell->GetName());
		return;
	}

	if (Spec.EffectShape == EDBAPlayableSkillEffectShape::HolyShield)
	{
		TSubclassOf<ADBAHolyShieldSpell> ShieldClass = Spec.HolyShieldClass ? Spec.HolyShieldClass : LobbyHolyShieldSpellClass;
		if (!ShieldClass)
		{
			ShieldClass = ADBAHolyShieldSpell::StaticClass();
		}

		ADBAHolyShieldSpell* ShieldSpell = GetWorld()->SpawnActor<ADBAHolyShieldSpell>(
			ShieldClass,
			GetActorLocation(),
			GetActorRotation(),
			SpawnParams);
		if (!ShieldSpell)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 生成牧师护盾法术失败：施法者=%s 技能=%s 类=%s"),
				*GetName(),
				*Spec.SkillId.ToString(),
				*GetNameSafe(ShieldClass));
			return;
		}

		AActor* ShieldTarget = Cast<ADBAZodiacCharacterBase>(TargetActor) ? TargetActor : this;
		ShieldSpell->CastHolyShield(this, ShieldTarget);
		SkillCooldowns[SkillSlot] = Spec.Cooldown;
		SkillMaxCooldowns[SkillSlot] = Spec.Cooldown;
		MulticastPlayLobbySkillCastFeedback(SkillSlot);
		UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacCharacterBase] 已施放牧师护盾：施法者=%s 技能=%s 目标=%s 法术=%s"),
			*GetName(),
			*Spec.SkillId.ToString(),
			*GetNameSafe(ShieldTarget),
			*ShieldSpell->GetName());
		return;
	}

	TSubclassOf<ADBASkillProjectileBase> ProjectileClass = LoadClass<ADBASkillProjectileBase>(
		nullptr,
		TEXT("/Game/DBA/Blueprints/Projectiles/BP_DBA_FireballProjectile.BP_DBA_FireballProjectile_C"));
	if (SkillSlot != 1 || !ProjectileClass)
	{
		if (SkillSlot == 1)
		{
			ProjectileClass = Spec.ProjectileClass ? Spec.ProjectileClass : LobbyFireballProjectileClass;
			if (!ProjectileClass)
			{
				ProjectileClass = ADBAFireballProjectile::StaticClass();
			}
		}
		else
		{
			if (SkillSlot == 2)
			{
				ProjectileClass = Spec.ProjectileClass ? Spec.ProjectileClass : LobbyFrostShardProjectileClass;
				if (!ProjectileClass)
				{
					ProjectileClass = ADBAFrostShardProjectile::StaticClass();
				}
			}
			else if (SkillSlot == 6)
			{
				ProjectileClass = Spec.ProjectileClass ? Spec.ProjectileClass : LobbyShadowBoltProjectileClass;
				if (!ProjectileClass)
				{
					ProjectileClass = ADBAShadowBoltProjectile::StaticClass();
				}
			}
			else
			{
				ProjectileClass = ADBAProjectile_Generic::StaticClass();
			}
		}
	}

	ADBASkillProjectileBase* Fireball = GetWorld()->SpawnActor<ADBASkillProjectileBase>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);
	if (!Fireball)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 生成装配技能投射物失败：槽位=%d 技能=%s 类=%s"),
			SkillSlot,
			*Spec.SkillId.ToString(),
			*GetNameSafe(ProjectileClass));
		return;
	}

	ApplyLobbySkillProjectileAssets(Fireball, Spec);
	Fireball->InitializeProjectile(Spec.SkillId, this, TargetActor, Spec.Magnitude, Spec.ProjectileSpeed, Spec.ProjectileRadius);
	Fireball->LaunchProjectile(SafeAimDirection);
	SkillCooldowns[SkillSlot] = Spec.Cooldown;
	SkillMaxCooldowns[SkillSlot] = Spec.Cooldown;
	MulticastPlayLobbySkillCastFeedback(SkillSlot);

	UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacCharacterBase] 已施放装配技能：施法者=%s 槽位=%d 技能=%s 投射物=%s 类=%s 目标=%s 水平方向=%s"),
		*GetName(),
		SkillSlot,
		*Spec.SkillId.ToString(),
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
	FDBAPlayableSkillRuntimeSpec Spec;
	if (!ResolvePlayableSkillSpec(this, SkillSlot, Spec))
	{
		return;
	}

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
	if (!Spec.CastNiagaraVFXAsset.IsNull())
	{
		if (UNiagaraSystem* CastVFX = Spec.CastNiagaraVFXAsset.LoadSynchronous())
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

	if (!Spec.CastSFXAsset.IsNull())
	{
		if (USoundBase* CastSFX = Spec.CastSFXAsset.LoadSynchronous())
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

TArray<FDBAPlayableSkillRuntimeSpec> ADBAZodiacCharacterBase::GetPlayableSkillSpecs() const
{
	return PlayableSkillComponent ? PlayableSkillComponent->GetAllSkillSpecs() : TArray<FDBAPlayableSkillRuntimeSpec>();
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
