// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊妯″瀷鍩虹被

#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/RPC/DBARpcHandler.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameDBA/Animation/DBAZodiacAnimInstance.h"
#include "Net/UnrealNetwork.h"

namespace
{
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
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	}

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
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

void ADBAZodiacCharacterBase::ApplyLobbyVisuals()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return;
	}

	const EDBAZodiac CommonZodiac = ToCommonZodiac(ZodiacType);
	const TArray<FString> MeshCandidates = {
		ADBACharacterPresentationActor::GetPreviewMeshPathForZodiac(CommonZodiac),
		ADBACharacterPresentationActor::GetPreviewLegacyMeshPathForZodiac(CommonZodiac),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat")
	};

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
	}

	MeshComponent->SetVisibility(true);
	MeshComponent->SetHiddenInGame(false);
	MeshComponent->SetComponentTickEnabled(true);
	MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	MeshComponent->SetBoundsScale(2.0f);
	MeshComponent->UpdateBounds();
	ADBACharacterPresentationActor::ApplyZodiacMaterialToMesh(MeshComponent, CommonZodiac, this);

	UE_LOG(LogTemp, Log, TEXT("[DBAZodiacCharacterBase] Lobby visuals applied: actor=%s zodiac=%d mesh=%s location=%s rotation=%s"),
		*GetName(),
		static_cast<int32>(CommonZodiac),
		ResolvedMeshPath.IsEmpty() ? TEXT("<unchanged>") : *ResolvedMeshPath,
		*MeshComponent->GetRelativeLocation().ToString(),
		*MeshComponent->GetRelativeRotation().ToString());
}

void ADBAZodiacCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UDBAZodiacAnimInstance* ADBAZodiacCharacterBase::GetZodiacAnimInstance() const
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		return Cast<UDBAZodiacAnimInstance>(MeshComp->GetAnimInstance());
	}
	return nullptr;
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
