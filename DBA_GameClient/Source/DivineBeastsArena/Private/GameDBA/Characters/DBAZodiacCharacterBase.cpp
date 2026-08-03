// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 生肖角色模型基类

#include "GameDBA/Characters/DBAZodiacCharacterBase.h"
#include "GameDBA/Gameplay/Abilities/Spells/DBABloomHealingSpell.h"
#include "GameDBA/Gameplay/Abilities/Spells/DBAChainLightningSpell.h"
#include "GameDBA/Gameplay/Abilities/Spells/DBAHolyShieldSpell.h"
#include "GameDBA/Gameplay/Loadout/DBAPlayableSkillComponent.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "GameDBA/Gameplay/Abilities/Projectiles/DBASkillProjectileBase.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Gameplay/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBAHeroGrowthAttributeSet.h"
#include "GameDBA/Framework/Replication/DBAPlayerState.h"
#include "GameDBA/Framework/Replication/RPC/DBARpcHandler.h"
#include "GameDBA/Gameplay/Loadout/SkillGroups/DBASkillGroupGeneratorSubsystem.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterPresentationActor.h"
#include "GameDBA/UI/Controllers/DBAGameUIManager.h"
#include "Camera/CameraComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameDBA/Presentation/Animation/DBAZodiacAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	bool ResolvePlayableSkillSpec(const ADBAZodiacCharacterBase* Character, int32 SkillSlot, FDBAPlayableSkillRuntimeSpec& OutSpec);

	bool IsLobbyEquippedSkillSlot(int32 SkillSlot)
	{
		return SkillSlot >= 1 && SkillSlot <= 6;
	}

	int32 MapEquippedSkillSlotToAbilityInputID(int32 SkillSlot)
	{
		switch (SkillSlot)
		{
		case 1: return static_cast<int32>(EDBAAbilityInputID::Skill01);
		case 2: return static_cast<int32>(EDBAAbilityInputID::Skill02);
		case 3: return static_cast<int32>(EDBAAbilityInputID::Skill03);
		case 4: return static_cast<int32>(EDBAAbilityInputID::Skill04);
		case 5: return static_cast<int32>(EDBAAbilityInputID::Ultimate);
		default: return static_cast<int32>(EDBAAbilityInputID::None);
		}
	}

	int32 MapArenaHUDSkillCueToAbilityInputID(FName SkillId)
	{
		if (SkillId == TEXT("Skill01"))
		{
			return static_cast<int32>(EDBAAbilityInputID::Skill01);
		}
		if (SkillId == TEXT("Skill02"))
		{
			return static_cast<int32>(EDBAAbilityInputID::Skill02);
		}
		if (SkillId == TEXT("Skill03"))
		{
			return static_cast<int32>(EDBAAbilityInputID::Skill03);
		}
		if (SkillId == TEXT("Skill04"))
		{
			return static_cast<int32>(EDBAAbilityInputID::Skill04);
		}
		if (SkillId == TEXT("Ultimate"))
		{
			return static_cast<int32>(EDBAAbilityInputID::Ultimate);
		}
		return static_cast<int32>(EDBAAbilityInputID::None);
	}

	int32 MapArenaHUDSkillCueToSkillSlot(FName SkillId)
	{
		switch (static_cast<EDBAAbilityInputID>(MapArenaHUDSkillCueToAbilityInputID(SkillId)))
		{
		case EDBAAbilityInputID::Skill01:
			return 1;
		case EDBAAbilityInputID::Skill02:
			return 2;
		case EDBAAbilityInputID::Skill03:
			return 3;
		case EDBAAbilityInputID::Skill04:
			return 4;
		case EDBAAbilityInputID::Ultimate:
			return 5;
		default:
			return INDEX_NONE;
		}
	}

	FText ResolveArenaHUDSkillCueFallbackDisplayName(FName SkillId)
	{
		switch (static_cast<EDBAAbilityInputID>(MapArenaHUDSkillCueToAbilityInputID(SkillId)))
		{
		case EDBAAbilityInputID::Skill01:
			return NSLOCTEXT("DBAArenaHUD", "SkillCueFallbackSkill01", "技能一");
		case EDBAAbilityInputID::Skill02:
			return NSLOCTEXT("DBAArenaHUD", "SkillCueFallbackSkill02", "技能二");
		case EDBAAbilityInputID::Skill03:
			return NSLOCTEXT("DBAArenaHUD", "SkillCueFallbackSkill03", "技能三");
		case EDBAAbilityInputID::Skill04:
			return NSLOCTEXT("DBAArenaHUD", "SkillCueFallbackSkill04", "技能四");
		case EDBAAbilityInputID::Ultimate:
			return NSLOCTEXT("DBAArenaHUD", "SkillCueFallbackUltimate", "终极技能");
		default:
			return NSLOCTEXT("DBAArenaHUD", "SkillCueFallbackUnknown", "技能");
		}
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

	EDBAZodiacType ToZodiacType(EDBAZodiac Zodiac)
	{
		const uint8 ZodiacValue = static_cast<uint8>(Zodiac);
		const uint8 LastZodiacValue = static_cast<uint8>(EDBAZodiac::Pig);
		return ZodiacValue <= LastZodiacValue
			? static_cast<EDBAZodiacType>(ZodiacValue)
			: EDBAZodiacType::None;
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
		FDBAPlayableSkillRuntimeSpec PlayableSpec;
		const FName DataAssetSkillId = ResolvePlayableSkillSpec(Character, SkillSlot, PlayableSpec)
			? PlayableSpec.SkillId
			: NAME_None;
		if (!Character || !Character->GetWorld())
		{
			return DataAssetSkillId;
		}

		UGameInstance* GameInstance = Character->GetWorld()->GetGameInstance();
		UDBASkillGroupGeneratorSubsystem* SkillGroups = GameInstance
			? GameInstance->GetSubsystem<UDBASkillGroupGeneratorSubsystem>()
			: nullptr;
		if (!SkillGroups)
		{
			return DataAssetSkillId;
		}

		FDBAZodiacElementFixedSkillGroupRow SkillGroup;
		if (!SkillGroups->GetSkillGroup(ToCommonZodiac(Character->GetZodiacType()), ToCommonElement(Character->GetElementType()), SkillGroup))
		{
			return DataAssetSkillId;
		}

		switch (SkillSlot)
		{
		case 1: return SkillGroup.ElementSkill1Id.IsNone() ? DataAssetSkillId : SkillGroup.ElementSkill1Id;
		case 2: return SkillGroup.ElementSkill2Id.IsNone() ? DataAssetSkillId : SkillGroup.ElementSkill2Id;
		case 3: return SkillGroup.ElementSkill3Id.IsNone() ? DataAssetSkillId : SkillGroup.ElementSkill3Id;
		case 4: return SkillGroup.ElementSkill4Id.IsNone() ? DataAssetSkillId : SkillGroup.ElementSkill4Id;
		case 5: return SkillGroup.ZodiacUltimateSkillId.IsNone() ? DataAssetSkillId : SkillGroup.ZodiacUltimateSkillId;
		default: return DataAssetSkillId;
		}
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
		Projectile->NiagaraParameters = Spec.NiagaraParameters;
		Projectile->ProjectileNiagaraVFXAsset = Spec.ProjectileNiagaraVFXAsset;
		Projectile->ImpactNiagaraVFXAsset = Spec.ImpactNiagaraVFXAsset;
		Projectile->FlySFXAsset = Spec.FlySFXAsset;
		Projectile->ImpactSFXAsset = Spec.ImpactSFXAsset;
	}

	bool IsUsablePresentationWorld(const UWorld* World)
	{
		return IsValid(World);
	}
}

ADBAZodiacCharacterBase::ADBAZodiacCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	// 配置碰撞
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

	// 配置移动速度
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
	SkillCooldowns.Init(0.0f, DBAConstants::PlayableSkillArraySize);
	SkillMaxCooldowns.Init(0.0f, DBAConstants::PlayableSkillArraySize);
}

void ADBAZodiacCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeDBAAbilityActorInfo();
}

void ADBAZodiacCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeDBAAbilityActorInfo();
}

void ADBAZodiacCharacterBase::InitializeDBAAbilityActorInfo()
{
	ADBAPlayerState* DBAPlayerState = GetPlayerState<ADBAPlayerState>();
	if (!DBAPlayerState)
	{
		UE_LOG(LogDBACombat, Verbose, TEXT("[生肖角色] 初始化 GAS 失败：PlayerState 尚未就绪。"));
		return;
	}

	UDBAAbilitySystemComponent* DBAAbilitySystem = DBAPlayerState->GetDBAAbilitySystemComponent();
	if (!DBAAbilitySystem)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[生肖角色] 初始化 GAS 失败：PlayerState 未持有 DBAAbilitySystemComponent。"));
		return;
	}

	DBAAbilitySystem->InitializeAbilities(DBAPlayerState, this);
	BindArenaHUDAttributeDelegates();
	SyncArenaHUDFromAttributes(true);
}

void ADBAZodiacCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeDBAAbilityActorInfo();
	ApplyLobbyVisuals();
	BindArenaHUDAttributeDelegates();
	SyncArenaHUDFromAttributes(true);

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

void ADBAZodiacCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindArenaHUDAttributeDelegates();
	Super::EndPlay(EndPlayReason);
}

void ADBAZodiacCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (LobbyAttackAnimationTimeRemaining > 0.0f)
	{
		LobbyAttackAnimationTimeRemaining = FMath::Max(0.0f, LobbyAttackAnimationTimeRemaining - DeltaSeconds);
	}

	UpdateLobbyLocomotionAnimation();
	BindArenaHUDAttributeDelegates();
	if (!bHasBoundArenaHUDAttributeDelegates)
	{
		SyncArenaHUDFromAttributes();
	}
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

		// 使用 TSoftObjectPtr 替代 LoadObject，资源需已预加载
		const FSoftObjectPath MeshSoftPath(MeshPath);
		const TSoftObjectPtr<USkeletalMesh> MeshRef(MeshSoftPath);
		ResolvedMesh = MeshRef.Get();
		if (ResolvedMesh)
		{
			ResolvedMeshPath = MeshPath;
			break;
		}
	}

	if (!ResolvedMesh && !bLobbyVisualLoadRequested && GetNetMode() != NM_DedicatedServer)
	{
		bLobbyVisualLoadRequested = true;
		TArray<FSoftObjectPath> AssetsToLoad;
		const TWeakObjectPtr<ADBAZodiacCharacterBase> WeakThis(this);
		for (const FString& MeshPath : MeshCandidates)
		{
			if (MeshPath.IsEmpty())
			{
				continue;
			}
			AssetsToLoad.AddUnique(FSoftObjectPath(MeshPath));
		}

		if (AssetsToLoad.IsEmpty())
		{
			bLobbyVisualLoadRequested = false;
			UE_LOG(LogDBAFrontend, Error, TEXT("[DBAZodiacCharacterBase] 大厅角色表现资源加载失败：生肖=%d 未提供有效的候选资源路径。"), static_cast<int32>(CommonZodiac));
		}
		else
		{
			UAssetManager::GetStreamableManager().RequestAsyncLoad(
				AssetsToLoad,
				FStreamableDelegate::CreateWeakLambda(this, [WeakThis]()
				{
					if (ADBAZodiacCharacterBase* StrongThis = WeakThis.Get())
					{
						StrongThis->bLobbyVisualLoadRequested = false;
						UE_LOG(LogDBAFrontend, Log, TEXT("[DBAZodiacCharacterBase] 大厅角色表现资源异步加载完成，开始应用模型与动画：Actor=%s。"), *StrongThis->GetName());
						StrongThis->ApplyLobbyVisuals();
					}
				}),
				FStreamableManager::AsyncLoadHighPriority,
				true);

			UE_LOG(LogDBAFrontend, Log, TEXT("[DBAZodiacCharacterBase] 大厅角色表现资源尚未驻留内存，已异步请求模型、骨骼与动画：生肖=%d 资源数量=%d。"),
				static_cast<int32>(CommonZodiac),
				AssetsToLoad.Num());
		}
	}

	if (ResolvedMesh)
	{
		MeshComponent->SetSkeletalMesh(ResolvedMesh);
		const float CapsuleHalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.0f;
		const FBox MeshBox = ResolvedMesh->GetBounds().GetBox();
		const float MeshBottomOffsetZ = MeshBox.IsValid ? -MeshBox.Min.Z + 2.0f : 0.0f;
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -CapsuleHalfHeight + MeshBottomOffsetZ));
		MeshComponent->SetRelativeRotation(ADBACharacterPresentationActor::GetPreviewMeshPlayerFacingRotation());
		// 大厅角色保持导入骨骼网格的原始比例，禁止在运行时额外放大。
		MeshComponent->SetRelativeScale3D(FVector::OneVector);

		if (ResolvedMesh->GetSkeleton())
		{
			if (!ADBACharacterPresentationActor::ApplyLobbyDisplayAnimationToMesh(MeshComponent, ResolvedMeshPath, CommonZodiac))
			{
				MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				MeshComponent->SetAnimInstanceClass(UDBAZodiacAnimInstance::StaticClass());
			}
		}
		else
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			UE_LOG(LogDBAFrontend, Warning, TEXT("[DBAZodiacCharacterBase] 网格没有骨骼，已跳过动画蓝图：%s"), *ResolvedMeshPath);
		}
	}

	MeshComponent->SetVisibility(true);
	MeshComponent->SetHiddenInGame(false);
	MeshComponent->SetComponentTickEnabled(true);
	MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	MeshComponent->SetBoundsScale(2.0f);
	MeshComponent->UpdateBounds();
	ADBACharacterPresentationActor::ApplyZodiacMaterialToMesh(MeshComponent, CommonZodiac, this);

	UE_LOG(LogDBAFrontend, Log, TEXT("[DBAZodiacCharacterBase] 大厅角色外观已应用：Actor=%s 生肖=%d 网格=%s 骨骼=%s 动画=%s 相对位置=%s 相对旋转=%s"),
		*GetName(),
		static_cast<int32>(CommonZodiac),
		ResolvedMeshPath.IsEmpty() ? TEXT("<unchanged>") : *ResolvedMeshPath,
		ResolvedMesh && ResolvedMesh->GetSkeleton() ? TEXT("是") : TEXT("否"),
		MeshComponent->GetAnimInstance() ? *MeshComponent->GetAnimInstance()->GetClass()->GetName() : *GetNameSafe(CurrentLobbyAnimation),
		*MeshComponent->GetRelativeLocation().ToString(),
		*MeshComponent->GetRelativeRotation().ToString());
}

void ADBAZodiacCharacterBase::SetLobbyDisplayZodiac(EDBAZodiac NewZodiac)
{
	if (NewZodiac == EDBAZodiac::None)
	{
		UE_LOG(LogDBAFrontend, Warning, TEXT("[DBAZodiacCharacterBase] 收到无效大厅生肖，拒绝覆盖现有外观：Actor=%s"), *GetName());
		return;
	}

	const EDBAZodiacType NewZodiacType = ToZodiacType(NewZodiac);
	if (NewZodiacType == EDBAZodiacType::None)
	{
		UE_LOG(LogDBAFrontend, Error, TEXT("[DBAZodiacCharacterBase] 大厅生肖转换失败：Actor=%s 生肖=%d"), *GetName(), static_cast<int32>(NewZodiac));
		return;
	}

	ZodiacType = NewZodiacType;
	UE_LOG(LogDBAFrontend, Log, TEXT("[DBAZodiacCharacterBase] 已设置大厅展示生肖：Actor=%s 生肖=%d"), *GetName(), static_cast<int32>(NewZodiac));
	ApplyLobbyVisuals();
}

void ADBAZodiacCharacterBase::OnRep_ZodiacType()
{
	UE_LOG(LogDBAFrontend, Log, TEXT("[DBAZodiacCharacterBase] 已同步大厅生肖，刷新本地外观：Actor=%s 生肖=%d"), *GetName(), static_cast<int32>(ZodiacType));
	ApplyLobbyVisuals();
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

	if (!HasAuthority() && !IsLocallyControlled())
	{
		return;
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

	if (!HasAuthority() && !IsLocallyControlled())
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerCastEquippedSkill(SkillSlot, TargetActor, AimDirection);
		return;
	}

	CastEquippedSkillInternal(SkillSlot, AimDirection, TargetActor);
}

bool ADBAZodiacCharacterBase::ValidateServerEquippedSkillCast(int32 SkillSlot, AActor* TargetActor) const
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!GetWorld())
	{
		return false;
	}

	if (!IsLobbyEquippedSkillSlot(SkillSlot))
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 服务端施法被拒绝：槽位无效，施法者=%s 槽位=%d"),
			*GetName(),
			SkillSlot);
		return false;
	}

	if (IsDead())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 服务端施法被拒绝：角色已死亡，施法者=%s 槽位=%d"),
			*GetName(),
			SkillSlot);
		return false;
	}

	if (TargetActor && !IsValid(TargetActor))
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 服务端施法被拒绝：目标无效，施法者=%s 槽位=%d"),
			*GetName(),
			SkillSlot);
		return false;
	}

	return true;
}

bool ADBAZodiacCharacterBase::ServerCastLobbyFireball_Validate(FVector_NetQuantizeNormal AimDirection)
{
	static_cast<void>(AimDirection);
	return ValidateServerEquippedSkillCast(1, nullptr);
}

void ADBAZodiacCharacterBase::ServerCastLobbyFireball_Implementation(FVector_NetQuantizeNormal AimDirection)
{
	if (!ValidateServerEquippedSkillCast(1, nullptr))
	{
		return;
	}

	CastEquippedSkillInternal(1, FVector(AimDirection));
}

bool ADBAZodiacCharacterBase::ServerCastLobbyFireballAtTarget_Validate(AActor* TargetActor, FVector_NetQuantizeNormal FallbackAimDirection)
{
	static_cast<void>(FallbackAimDirection);
	return ValidateServerEquippedSkillCast(1, TargetActor);
}

void ADBAZodiacCharacterBase::ServerCastLobbyFireballAtTarget_Implementation(AActor* TargetActor, FVector_NetQuantizeNormal FallbackAimDirection)
{
	if (!ValidateServerEquippedSkillCast(1, TargetActor))
	{
		return;
	}

	CastEquippedSkillInternal(1, FVector(FallbackAimDirection), TargetActor);
}

bool ADBAZodiacCharacterBase::ServerCastEquippedSkill_Validate(int32 SkillSlot, AActor* TargetActor, FVector_NetQuantizeNormal FallbackAimDirection)
{
	static_cast<void>(FallbackAimDirection);
	return ValidateServerEquippedSkillCast(SkillSlot, TargetActor);
}

void ADBAZodiacCharacterBase::ServerCastEquippedSkill_Implementation(int32 SkillSlot, AActor* TargetActor, FVector_NetQuantizeNormal FallbackAimDirection)
{
	if (!ValidateServerEquippedSkillCast(SkillSlot, TargetActor))
	{
		return;
	}

	CastEquippedSkillInternal(SkillSlot, FVector(FallbackAimDirection), TargetActor);
}

void ADBAZodiacCharacterBase::CastLobbyFireballInternal(const FVector& AimDirection, AActor* TargetActor)
{
	CastEquippedSkillInternal(1, AimDirection, TargetActor);
}

void ADBAZodiacCharacterBase::CastEquippedSkillInternal(int32 SkillSlot, const FVector& AimDirection, AActor* TargetActor)
{
	if (!HasAuthority())
	{
		return;
	}

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

	const int32 AbilityInputID = MapEquippedSkillSlotToAbilityInputID(SkillSlot);
	if (AbilityInputID != static_cast<int32>(EDBAAbilityInputID::None))
	{
		if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
		{
			if (ASC->TryActivateAbilityByInputID(AbilityInputID, TargetActor))
			{
				return;
			}
		}
	}

	FDBAPlayableSkillRuntimeSpec Spec;
	if (!ResolvePlayableSkillSpec(this, SkillSlot, Spec))
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 未找到装配技能规格：施法者=%s 槽位=%d"),
			*GetName(),
			SkillSlot);
		return;
	}

	if (SkillCooldowns.Num() < DBAConstants::PlayableSkillArraySize)
	{
		SkillCooldowns.SetNumZeroed(DBAConstants::PlayableSkillArraySize);
	}
	if (SkillMaxCooldowns.Num() < DBAConstants::PlayableSkillArraySize)
	{
		SkillMaxCooldowns.SetNumZeroed(DBAConstants::PlayableSkillArraySize);
	}
	const int32 CooldownArrayIndex = SkillSlot - 1;
	if (!SkillCooldowns.IsValidIndex(CooldownArrayIndex) || !SkillMaxCooldowns.IsValidIndex(CooldownArrayIndex))
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 装配技能冷却索引无效：施法者=%s 槽位=%d 索引=%d"),
			*GetName(),
			SkillSlot,
			CooldownArrayIndex);
		return;
	}

	SkillMaxCooldowns[CooldownArrayIndex] = FMath::Max(SkillMaxCooldowns[CooldownArrayIndex], Spec.Cooldown);
	if (SkillCooldowns.IsValidIndex(CooldownArrayIndex) && SkillCooldowns[CooldownArrayIndex] > 0.0f)
	{
		UE_LOG(LogDBACombat, Verbose, TEXT("[DBAZodiacCharacterBase] 装配技能被冷却阻止：施法者=%s 槽位=%d 技能=%s 剩余=%.2f"),
			*GetName(),
			SkillSlot,
			*Spec.SkillId.ToString(),
			SkillCooldowns[CooldownArrayIndex]);
		return;
	}

	const auto ApplyLegacyCooldown = [this, CooldownArrayIndex, &Spec]()
	{
		SkillCooldowns[CooldownArrayIndex] = Spec.Cooldown;
		SkillMaxCooldowns[CooldownArrayIndex] = Spec.Cooldown;
		OnSkillCooldownsChanged.Broadcast(SkillCooldowns);
	};

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
		TSubclassOf<ADBABloomHealingSpell> BloomClass = Spec.BloomHealingClass;
		if (!BloomClass)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 绽放治疗技能缺少数据资产配置类：施法者=%s 技能=%s"),
				*GetName(),
				*Spec.SkillId.ToString());
			return;
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
		MulticastPlayLobbySkillCastFeedback(SkillSlot);
		BloomSpell->ConfigureFromSkillSpec(Spec);
		BloomSpell->CastBloomHealing(this, HealTarget);
		ApplyLegacyCooldown();
		UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacCharacterBase] 已施放绽放治疗：施法者=%s 技能=%s 目标=%s 法术=%s"),
			*GetName(),
			*Spec.SkillId.ToString(),
			*GetNameSafe(HealTarget),
			*BloomSpell->GetName());
		return;
	}

	if (Spec.EffectShape == EDBAPlayableSkillEffectShape::ChainLightning)
	{
		TSubclassOf<ADBAChainLightningSpell> ChainClass = Spec.ChainLightningClass;
		if (!ChainClass)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 链式闪电技能缺少数据资产配置类：施法者=%s 技能=%s"),
				*GetName(),
				*Spec.SkillId.ToString());
			return;
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

		MulticastPlayLobbySkillCastFeedback(SkillSlot);
		ChainSpell->ConfigureFromSkillSpec(Spec);
		ChainSpell->CastChainLightning(this, TargetActor);
		ApplyLegacyCooldown();
		UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacCharacterBase] 已施放链式闪电：施法者=%s 技能=%s 初始目标=%s 法术=%s"),
			*GetName(),
			*Spec.SkillId.ToString(),
			*GetNameSafe(TargetActor),
			*ChainSpell->GetName());
		return;
	}

	if (Spec.EffectShape == EDBAPlayableSkillEffectShape::HolyShield)
	{
		TSubclassOf<ADBAHolyShieldSpell> ShieldClass = Spec.HolyShieldClass;
		if (!ShieldClass)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 牧师护盾技能缺少数据资产配置类：施法者=%s 技能=%s"),
				*GetName(),
				*Spec.SkillId.ToString());
			return;
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
		MulticastPlayLobbySkillCastFeedback(SkillSlot);
		ShieldSpell->ConfigureFromSkillSpec(Spec);
		ShieldSpell->CastHolyShield(this, ShieldTarget);
		ApplyLegacyCooldown();
		UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacCharacterBase] 已施放牧师护盾：施法者=%s 技能=%s 目标=%s 法术=%s"),
			*GetName(),
			*Spec.SkillId.ToString(),
			*GetNameSafe(ShieldTarget),
			*ShieldSpell->GetName());
		return;
	}

	TSubclassOf<ADBASkillProjectileBase> ProjectileClass = Spec.ProjectileClass;
	if (!ProjectileClass)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacCharacterBase] 投射物技能缺少数据资产配置类：施法者=%s 槽位=%d 技能=%s"),
			*GetName(),
			SkillSlot,
			*Spec.SkillId.ToString());
		return;
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
	MulticastPlayLobbySkillCastFeedback(SkillSlot);
	Fireball->InitializeProjectile(Spec.SkillId, this, TargetActor, Spec.Magnitude, Spec.ProjectileSpeed, Spec.ProjectileRadius);
	Fireball->LaunchProjectile(SafeAimDirection);
	ApplyLegacyCooldown();

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

	const FVector CastRelativeLocation(72.0f, 0.0f, 88.0f);
	const FVector CastLocation = GetActorLocation() + GetActorForwardVector() * CastRelativeLocation.X + FVector(0.0f, 0.0f, CastRelativeLocation.Z);
	if (!Spec.CastNiagaraVFXAsset.IsNull())
	{
		if (UNiagaraSystem* CastVFX = Spec.CastNiagaraVFXAsset.Get())
		{
			if (GetRootComponent())
			{
				UNiagaraComponent* CastComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
					CastVFX,
					GetRootComponent(),
					NAME_None,
					CastRelativeLocation,
					FRotator::ZeroRotator,
					FVector(Spec.CastVFXScale),
					EAttachLocation::KeepRelativeOffset,
					true,
					ENCPoolMethod::AutoRelease,
					true,
					true);
				UDBANiagaraSkillParameterLibrary::ApplySkillParameters(
					CastComponent,
					Spec.NiagaraParameters,
					Spec.Magnitude,
					CastLocation,
					GetActorForwardVector(),
					Spec.ProjectileSpeed,
					Spec.ProjectileRadius);
			}
		}
		else
		{
			TWeakObjectPtr<ADBAZodiacCharacterBase> WeakThis(this);
			const FVector RelativeLocation = CastRelativeLocation;
			const FVector Scale(Spec.CastVFXScale);
			const FDBAPlayableSkillRuntimeSpec CapturedSpec = Spec;
			DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(this, Spec.CastNiagaraVFXAsset, [WeakThis, RelativeLocation, Scale, CapturedSpec](UNiagaraSystem* LoadedVFX)
			{
				ADBAZodiacCharacterBase* StrongThis = WeakThis.Get();
				if (!StrongThis || !LoadedVFX || StrongThis->GetNetMode() == NM_DedicatedServer)
				{
					return;
				}

				if (USceneComponent* Root = StrongThis->GetRootComponent())
				{
					UNiagaraComponent* CastComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
						LoadedVFX,
						Root,
						NAME_None,
						RelativeLocation,
						FRotator::ZeroRotator,
						Scale,
						EAttachLocation::KeepRelativeOffset,
						true,
						ENCPoolMethod::AutoRelease,
						true,
						true);
					const FVector CastWorldLocation = StrongThis->GetActorLocation() + StrongThis->GetActorForwardVector() * RelativeLocation.X + FVector(0.0f, 0.0f, RelativeLocation.Z);
					UDBANiagaraSkillParameterLibrary::ApplySkillParameters(
						CastComponent,
						CapturedSpec.NiagaraParameters,
						CapturedSpec.Magnitude,
						CastWorldLocation,
						StrongThis->GetActorForwardVector(),
						CapturedSpec.ProjectileSpeed,
						CapturedSpec.ProjectileRadius);
				}
			});
		}
	}

	if (!Spec.CastSFXAsset.IsNull())
	{
		if (USoundBase* CastSFX = Spec.CastSFXAsset.Get())
		{
			if (GetRootComponent())
			{
				UGameplayStatics::SpawnSoundAttached(CastSFX, GetRootComponent(), NAME_None, CastRelativeLocation, EAttachLocation::KeepRelativeOffset, true, 0.85f);
			}
			else
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), CastSFX, CastLocation, 0.85f);
			}
		}
		else
		{
			TWeakObjectPtr<ADBAZodiacCharacterBase> WeakThis(this);
			const FVector RelativeLocation = CastRelativeLocation;
			const FVector WorldLocation = CastLocation;
			DBAAsyncAssetLoader::RequestAsyncAsset<USoundBase>(this, Spec.CastSFXAsset, [WeakThis, RelativeLocation, WorldLocation](USoundBase* LoadedSFX)
			{
				ADBAZodiacCharacterBase* StrongThis = WeakThis.Get();
				if (!StrongThis || !LoadedSFX || StrongThis->GetNetMode() == NM_DedicatedServer)
				{
					return;
				}

				if (USceneComponent* Root = StrongThis->GetRootComponent())
				{
					UGameplayStatics::SpawnSoundAttached(LoadedSFX, Root, NAME_None, RelativeLocation, EAttachLocation::KeepRelativeOffset, true, 0.85f);
				}
				else if (IsUsablePresentationWorld(StrongThis->GetWorld()))
				{
					UGameplayStatics::PlaySoundAtLocation(StrongThis->GetWorld(), LoadedSFX, WorldLocation, 0.85f);
				}
			});
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

	UE_LOG(LogDBAFrontend, Log, TEXT("[DBAZodiacCharacterBase] 已应用大厅单节点动画：Actor=%s 动画=%s 循环=%s 速度=%s"),
		*GetName(),
		*DesiredAnimation->GetPathName(),
		bPlayingAttack ? TEXT("否") : TEXT("是"),
		*GetVelocity().ToString());
}

UDBAAbilitySystemComponent* ADBAZodiacCharacterBase::GetDBAAbilitySystemComponent() const
{
	if (ADBAPlayerState* DBAPlayerState = GetPlayerState<ADBAPlayerState>())
	{
		return DBAPlayerState->GetDBAAbilitySystemComponent();
	}

	if (UDBAAbilitySystemComponent* LocalASC = FindComponentByClass<UDBAAbilitySystemComponent>())
	{
		return LocalASC;
	}

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

// ==================== 属性访问实现 ====================

float ADBAZodiacCharacterBase::GetCurrentHealth() const
{
	// 重写基类 ADBACharacterBase 的默认实现，从 GAS AttributeSet 读取真实生命值
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetNumericAttributeBase(UDBABattleAttributeSet::GetCurrentHealthAttribute());
	}
	return 0.0f;
}

float ADBAZodiacCharacterBase::GetMaxHealth() const
{
	// 重写基类 ADBACharacterBase 的默认实现，从 GAS AttributeSet 读取真实最大生命值
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

int32 ADBAZodiacCharacterBase::GetHeroLevel() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return FMath::Max(1, FMath::RoundToInt(ASC->GetNumericAttributeBase(UDBAHeroGrowthAttributeSet::GetHeroLevelAttribute())));
	}
	return 1;
}

float ADBAZodiacCharacterBase::GetUltimateEnergy() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetUltimateEnergy();
	}
	// P0-3 修复：原回退读取已删除的 Replicated 字段，改为返回默认值 0
	return 0.0f;
}

int32 ADBAZodiacCharacterBase::GetChainLevel() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetChainLevel();
	}
	// P0-3 修复：原回退读取已删除的 Replicated 字段，改为返回默认值 0
	return 0;
}

int32 ADBAZodiacCharacterBase::GetResonanceLevel() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetResonanceLevel();
	}
	// P0-3 修复：原回退读取已删除的 Replicated 字段，改为返回默认值 0
	return 0;
}

void ADBAZodiacCharacterBase::SyncArenaHUDFromAttributes(bool bForce)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
	{
		UIManager->BindArenaHUDToCharacter(this);

		const float CurrentHP = GetCurrentHealth();
		const float MaxHP = GetMaxHealth();
		const float CurrentEnergy = GetCurrentEnergy();
		const float MaxEnergy = GetMaxEnergy();
		const float CurrentUltimateEnergy = GetUltimateEnergy();
		const float MaxUltimateEnergy = DBAConstants::MaxUltimateEnergy;
		const int32 HeroLevel = GetHeroLevel();
		const int32 CurrentChainLevel = GetChainLevel();
		const int32 CurrentResonanceLevel = GetResonanceLevel();
		const float HealthRatio = MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f;
		const float EnergyRatio = MaxEnergy > 0.0f ? CurrentEnergy / MaxEnergy : 0.0f;
		const bool bLowHP = HealthRatio <= ArenaHUDCriticalHealthRatioThreshold;
		const bool bLowEnergy = EnergyRatio <= ArenaHUDCriticalEnergyRatioThreshold;
		const bool bUltimateReady = CurrentUltimateEnergy >= MaxUltimateEnergy;
		const bool bChainReady = CurrentChainLevel >= DBAConstants::MaxChainLevel;

		const bool bVitalsChanged =
			!FMath::IsNearlyEqual(CurrentHP, LastSyncedArenaHUDCurrentHP, KINDA_SMALL_NUMBER) ||
			!FMath::IsNearlyEqual(MaxHP, LastSyncedArenaHUDMaxHP, KINDA_SMALL_NUMBER) ||
			!FMath::IsNearlyEqual(CurrentEnergy, LastSyncedArenaHUDCurrentEnergy, KINDA_SMALL_NUMBER) ||
			!FMath::IsNearlyEqual(MaxEnergy, LastSyncedArenaHUDMaxEnergy, KINDA_SMALL_NUMBER);
		const bool bUltimateEnergyChanged =
			!FMath::IsNearlyEqual(CurrentUltimateEnergy, LastSyncedArenaHUDUltimateEnergy, KINDA_SMALL_NUMBER) ||
			!FMath::IsNearlyEqual(MaxUltimateEnergy, LastSyncedArenaHUDMaxUltimateEnergy, KINDA_SMALL_NUMBER);
		const bool bLevelChanged = HeroLevel != LastSyncedArenaHUDHeroLevel;
		const bool bCombatStateChanged =
			CurrentChainLevel != LastSyncedArenaHUDChainLevel ||
			CurrentResonanceLevel != LastSyncedArenaHUDResonanceLevel;
		const bool bCriticalStateChanged =
			bLowHP != LastSyncedArenaHUDBLowHP ||
			bLowEnergy != LastSyncedArenaHUDBLowEnergy;
		const bool bUltimateReadyPromptChanged =
			bUltimateReady != bLastSyncedArenaHUDUltimateReady;

		if (bForce || !bHasSyncedArenaHUDAttributes || bVitalsChanged)
		{
			UIManager->UpdateArenaHUDPlayerVitals(CurrentHP, MaxHP, CurrentEnergy, MaxEnergy);
			LastSyncedArenaHUDCurrentHP = CurrentHP;
			LastSyncedArenaHUDMaxHP = MaxHP;
			LastSyncedArenaHUDCurrentEnergy = CurrentEnergy;
			LastSyncedArenaHUDMaxEnergy = MaxEnergy;
		}

		if (bForce || !bHasSyncedArenaHUDAttributes || bUltimateEnergyChanged)
		{
			UIManager->UpdateArenaHUDUltimateEnergy(CurrentUltimateEnergy, MaxUltimateEnergy);
			LastSyncedArenaHUDUltimateEnergy = CurrentUltimateEnergy;
			LastSyncedArenaHUDMaxUltimateEnergy = MaxUltimateEnergy;
		}

		if (bForce || !bHasSyncedArenaHUDUltimateReadyPrompt || bUltimateReadyPromptChanged)
		{
			if (bUltimateReady)
			{
				UIManager->ShowArenaHUDUltimateReadyPrompt();
			}
			else
			{
				UIManager->HideArenaHUDUltimateReadyPrompt();
			}
			bLastSyncedArenaHUDUltimateReady = bUltimateReady;
			bHasSyncedArenaHUDUltimateReadyPrompt = true;
		}

		if (bForce || !bHasSyncedArenaHUDAttributes || bLevelChanged)
		{
			UIManager->UpdateArenaHUDPlayerLevel(HeroLevel);
			LastSyncedArenaHUDHeroLevel = HeroLevel;
		}

		if (bForce || !bHasSyncedArenaHUDAttributes || bCombatStateChanged)
		{
			UIManager->UpdateArenaHUDCombatState(CurrentChainLevel, CurrentResonanceLevel);
			LastSyncedArenaHUDChainLevel = CurrentChainLevel;
			LastSyncedArenaHUDResonanceLevel = CurrentResonanceLevel;
		}

		if (bChainReady && !bLastSyncedArenaHUDChainReady)
		{
			UIManager->ShowArenaHUDCombatAnnouncement(NSLOCTEXT("DBAArenaHUD", "ChainReadyAnnouncement", "连锁就绪"), ArenaHUDChainReadyAnnouncementDuration);
		}
		bLastSyncedArenaHUDChainReady = bChainReady;

		if (bForce || !bHasSyncedArenaHUDCriticalState || bCriticalStateChanged)
		{
			UIManager->UpdateArenaHUDCriticalStateHints(bLowHP, bLowEnergy);
			LastSyncedArenaHUDBLowHP = bLowHP;
			LastSyncedArenaHUDBLowEnergy = bLowEnergy;
			bHasSyncedArenaHUDCriticalState = true;
		}

		bHasSyncedArenaHUDAttributes = true;
	}
}

void ADBAZodiacCharacterBase::BindArenaHUDAttributeDelegates()
{
	if (bHasBoundArenaHUDAttributeDelegates || !IsLocallyControlled())
	{
		return;
	}

	UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	ArenaHUDAttributeDelegateASC = ASC;
	ArenaHUDCurrentHealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDBABattleAttributeSet::GetCurrentHealthAttribute())
		.AddUObject(this, &ADBAZodiacCharacterBase::HandleArenaHUDAttributeChanged);
	ArenaHUDMaxHealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDBABattleAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &ADBAZodiacCharacterBase::HandleArenaHUDAttributeChanged);
	ArenaHUDCurrentEnergyChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDBABattleAttributeSet::GetCurrentEnergyAttribute())
		.AddUObject(this, &ADBAZodiacCharacterBase::HandleArenaHUDAttributeChanged);
	ArenaHUDMaxEnergyChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDBABattleAttributeSet::GetMaxEnergyAttribute())
		.AddUObject(this, &ADBAZodiacCharacterBase::HandleArenaHUDAttributeChanged);
	ArenaHUDHeroLevelChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDBAHeroGrowthAttributeSet::GetHeroLevelAttribute())
		.AddUObject(this, &ADBAZodiacCharacterBase::HandleArenaHUDAttributeChanged);
	ASC->OnUltimateEnergyChanged.RemoveDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDUltimateEnergyChanged);
	ASC->OnUltimateEnergyChanged.AddDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDUltimateEnergyChanged);
	ASC->OnChainLevelChanged.RemoveDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDChainLevelChanged);
	ASC->OnChainLevelChanged.AddDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDChainLevelChanged);
	ASC->OnResonanceLevelChanged.RemoveDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDResonanceLevelChanged);
	ASC->OnResonanceLevelChanged.AddDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDResonanceLevelChanged);
	ASC->OnSkillCueExecuted.RemoveDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDSkillCueExecuted);
	ASC->OnSkillCueExecuted.AddDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDSkillCueExecuted);
	bHasBoundArenaHUDAttributeDelegates = true;
}

void ADBAZodiacCharacterBase::UnbindArenaHUDAttributeDelegates()
{
	UDBAAbilitySystemComponent* ASC = ArenaHUDAttributeDelegateASC.Get();
	if (ASC)
	{
		if (ArenaHUDCurrentHealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UDBABattleAttributeSet::GetCurrentHealthAttribute()).Remove(ArenaHUDCurrentHealthChangedHandle);
		}
		if (ArenaHUDMaxHealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UDBABattleAttributeSet::GetMaxHealthAttribute()).Remove(ArenaHUDMaxHealthChangedHandle);
		}
		if (ArenaHUDCurrentEnergyChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UDBABattleAttributeSet::GetCurrentEnergyAttribute()).Remove(ArenaHUDCurrentEnergyChangedHandle);
		}
		if (ArenaHUDMaxEnergyChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UDBABattleAttributeSet::GetMaxEnergyAttribute()).Remove(ArenaHUDMaxEnergyChangedHandle);
		}
		if (ArenaHUDHeroLevelChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UDBAHeroGrowthAttributeSet::GetHeroLevelAttribute()).Remove(ArenaHUDHeroLevelChangedHandle);
		}
		ASC->OnUltimateEnergyChanged.RemoveDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDUltimateEnergyChanged);
		ASC->OnChainLevelChanged.RemoveDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDChainLevelChanged);
		ASC->OnResonanceLevelChanged.RemoveDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDResonanceLevelChanged);
		ASC->OnSkillCueExecuted.RemoveDynamic(this, &ADBAZodiacCharacterBase::HandleArenaHUDSkillCueExecuted);
	}

	ArenaHUDCurrentHealthChangedHandle.Reset();
	ArenaHUDMaxHealthChangedHandle.Reset();
	ArenaHUDCurrentEnergyChangedHandle.Reset();
	ArenaHUDMaxEnergyChangedHandle.Reset();
	ArenaHUDHeroLevelChangedHandle.Reset();
	ArenaHUDAttributeDelegateASC.Reset();
	bHasBoundArenaHUDAttributeDelegates = false;
}

void ADBAZodiacCharacterBase::HandleArenaHUDAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	SyncArenaHUDFromAttributes();
}

void ADBAZodiacCharacterBase::HandleArenaHUDUltimateEnergyChanged(float CurrentEnergy, float MaxEnergy)
{
	SyncArenaHUDFromAttributes();
}

void ADBAZodiacCharacterBase::HandleArenaHUDChainLevelChanged(int32 InChainLevel)
{
	SyncArenaHUDFromAttributes();
}

void ADBAZodiacCharacterBase::HandleArenaHUDResonanceLevelChanged(int32 InResonanceLevel)
{
	SyncArenaHUDFromAttributes();
}

void ADBAZodiacCharacterBase::HandleArenaHUDSkillCueExecuted(FName SkillId, AActor* Target)
{
	(void)Target;

	if (SkillId.IsNone())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UDBAGameUIManager* UIManager = GameInstance ? GameInstance->GetSubsystem<UDBAGameUIManager>() : nullptr;
	if (!UIManager)
	{
		return;
	}

	const FText AnnouncementText = FText::Format(
		NSLOCTEXT("DBAArenaHUD", "SkillCueAnnouncement", "{0} 已释放"),
		ResolveArenaHUDSkillCueDisplayName(SkillId));
	UIManager->ShowArenaHUDCombatAnnouncement(AnnouncementText, ArenaHUDSkillCueAnnouncementDuration);
	UIManager->AddArenaHUDEventFeedEntry(AnnouncementText, ArenaHUDSkillCueAnnouncementDuration);
}

FText ADBAZodiacCharacterBase::ResolveArenaHUDSkillCueDisplayName(FName SkillId) const
{
	const int32 AbilityInputID = MapArenaHUDSkillCueToAbilityInputID(SkillId);
	if (const UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		if (const FDBAAbilityRuntimeConfig* RuntimeConfig = ASC->FindAbilityRuntimeConfigByInputID(AbilityInputID))
		{
			if (!RuntimeConfig->DisplayName.IsEmpty())
			{
				return RuntimeConfig->DisplayName;
			}
		}
	}

	const int32 SkillSlot = MapArenaHUDSkillCueToSkillSlot(SkillId);
	if (SkillSlot != INDEX_NONE)
	{
		for (const FDBAPlayableSkillRuntimeSpec& SkillSpec : GetPlayableSkillSpecs())
		{
			if (SkillSpec.SkillSlot == SkillSlot && !SkillSpec.DisplayName.IsEmpty())
			{
				return SkillSpec.DisplayName;
			}
		}
	}

	return ResolveArenaHUDSkillCueFallbackDisplayName(SkillId);
}

void ADBAZodiacCharacterBase::SetUltimateEnergy(float Value)
{
	if (!HasAuthority())
	{
		return;
	}

	UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[生肖角色] 设置终极能量失败：能力系统组件不可用。"));
		return;
	}

	const float TargetEnergy = FMath::Clamp(Value, 0.0f, DBAConstants::MaxUltimateEnergy);
	const float DeltaEnergy = TargetEnergy - ASC->GetUltimateEnergy();
	if (DeltaEnergy > KINDA_SMALL_NUMBER)
	{
		ASC->AddUltimateEnergy(DeltaEnergy);
	}
	else if (DeltaEnergy < -KINDA_SMALL_NUMBER)
	{
		ASC->ConsumeUltimateEnergy(-DeltaEnergy);
	}
}

void ADBAZodiacCharacterBase::AddUltimateEnergy(float Delta)
{
	if (!HasAuthority())
	{
		return;
	}

	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		ASC->AddUltimateEnergy(Delta);
	}
	else
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[生肖角色] 增加终极能量失败：能力系统组件不可用。"));
	}
}

bool ADBAZodiacCharacterBase::IsUltimateReady() const
{
	return GetUltimateEnergy() >= DBAConstants::MaxUltimateEnergy;
}

void ADBAZodiacCharacterBase::AddChainLevel(int32 Delta)
{
	if (!HasAuthority())
	{
		return;
	}

	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		ASC->AddChainLevel(Delta);
	}
	else
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[生肖角色] 增加连锁等级失败：能力系统组件不可用。"));
	}
}

void ADBAZodiacCharacterBase::ResetChainLevel()
{
	if (!HasAuthority())
	{
		return;
	}

	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		ASC->ResetChainLevel();
	}
	else
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[生肖角色] 重置连锁等级失败：能力系统组件不可用。"));
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

// ==================== 死亡状态实现 ====================

void ADBAZodiacCharacterBase::OnDeath()
{
	if (!HasAuthority())
	{
		return;
	}

	if (IsDead())
	{
		return;
	}

	DeathState = EDADeathState::Dying;
	PlayDeathAnimation();

	// 延迟设置为 Dead 状态，等待死亡动画播放
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathStateFinalizeTimerHandle);
		DeathStateFinalizeTimerHandle = World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (!HasAuthority() || DeathState != EDADeathState::Dying)
			{
				return;
			}

			DeathState = EDADeathState::Dead;
		}));
	}
	else
	{
		DeathState = EDADeathState::Dead;
	}
}

void ADBAZodiacCharacterBase::OnRevive()
{
	if (!HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathStateFinalizeTimerHandle);
	}

	DeathState = EDADeathState::Alive;
	// Revive health is controlled by the caller.
}

void ADBAZodiacCharacterBase::UpdateSkillCooldowns(const TArray<float>& NewCooldowns)
{
	if (!HasAuthority())
	{
		return;
	}

	SkillCooldowns = NewCooldowns;

	// 更新最大冷却数组（如果比当前记录的更大）
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

	OnSkillCooldownsChanged.Broadcast(SkillCooldowns);
}

void ADBAZodiacCharacterBase::OnRep_SkillCooldowns()
{
	OnSkillCooldownsChanged.Broadcast(SkillCooldowns);
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
	OutData.MaxEnergy = GetMaxEnergy();
	OutData.UltimateEnergy = GetUltimateEnergy();
	OutData.bUltimateReady = IsUltimateReady();
	OutData.SkillCooldowns = SkillCooldowns;
	OutData.SkillMaxCooldowns = SkillMaxCooldowns;
}

void ADBAZodiacCharacterBase::SetTeamID(int32 NewTeamID)
{
	// 重写基类 ADBACharacterBase::SetTeamID，增加非负约束（基类仅做权限检查后直接赋值）
	if (!HasAuthority())
	{
		return;
	}

	TeamID = FMath::Max(0, NewTeamID);
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

bool ADBAZodiacCharacterBase::IsDead() const
{
	// 转发到基类实现，同时满足 IIDBACharacterRef 接口的纯虚约定，消解“接口同名函数 + 基类同名函数”的二义性。
	return ADBACharacterBase::IsDead();
}

int32 ADBAZodiacCharacterBase::GetTeamID() const
{
	// 转发到基类实现，同时满足 IIDBACharacterRef 接口的纯虚约定，消解“接口同名函数 + 基类同名函数”的二义性。
	return ADBACharacterBase::GetTeamID();
}

float ADBAZodiacCharacterBase::GetMaxEnergy() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetNumericAttributeBase(UDBABattleAttributeSet::GetMaxEnergyAttribute());
	}
	return 0.0f;
}

bool ADBAZodiacCharacterBase::IsAbilityOnCooldown(FName SkillId) const
{
	if (SkillId.IsNone())
	{
		return false;
	}

	const TArray<FDBAPlayableSkillRuntimeSpec> PlayableSkillSpecs = GetPlayableSkillSpecs();
	for (const FDBAPlayableSkillRuntimeSpec& SkillSpec : PlayableSkillSpecs)
	{
		if (SkillSpec.SkillId == SkillId)
		{
			const int32 CooldownArrayIndex = SkillSpec.SkillSlot - 1;
			return SkillCooldowns.IsValidIndex(CooldownArrayIndex) && SkillCooldowns[CooldownArrayIndex] > 0.0f;
		}
	}

	return false;
}

bool ADBAZodiacCharacterBase::HasEnoughEnergy(float Cost) const
{
	return GetCurrentEnergy() >= Cost;
}




void ADBAZodiacCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// P0-3 修复：已移除 UltimateEnergy / ChainLevel / ResonanceLevel 的 DOREPLIFETIME_CONDITION（字段已删除）
	// P1-6 改造：DeathState 和 TeamID 已上移到基类 ADBACharacterBase，其 GetLifetimeReplicatedProps 已注册 DOREPLIFETIME
	DOREPLIFETIME(ADBAZodiacCharacterBase, HeroID);
	DOREPLIFETIME(ADBAZodiacCharacterBase, ZodiacType);
	DOREPLIFETIME_CONDITION(ADBAZodiacCharacterBase, SkillCooldowns, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ADBAZodiacCharacterBase, SkillMaxCooldowns, COND_OwnerOnly);
}
