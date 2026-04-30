// Copyright FreeboozStudio. All Rights Reserved.

#include "MobaBase/Input/DBAInputRouterComponent.h"
#include "MobaBase/Data/DBAInputConfigDataAsset.h"
#include "MobaBase/Input/DBAInputPlatformPolicy.h"
#include "Common/DBALogChannels.h"
#include "Common/DBAInterfacesCore.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/LocalPlayer.h"
#include "Net/UnrealNetwork.h"

UDBAInputRouterComponent::UDBAInputRouterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Dedicated Server 不创建此组件
#if UE_SERVER
	bAutoActivate = false;
#endif
}

void UDBAInputRouterComponent::BeginPlay()
{
	Super::BeginPlay();

	// Dedicated Server 不执行逻辑
#if UE_SERVER
	SetComponentTickEnabled(false);
	return;
#endif
}

void UDBAInputRouterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理输入映射上下文
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem())
	{
		Subsystem->ClearAllMappings();
	}

	Super::EndPlay(EndPlayReason);
}

void UDBAInputRouterComponent::InitializeInputRouter(UDBAInputConfigDataAsset* InputConfig, UDBAInputPlatformPolicy* PlatformPolicy)
{
	if (!InputConfig || !PlatformPolicy)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[DBAInputRouter] InitializeInputRouter 失败：InputConfig 或 PlatformPolicy 为空"));
		return;
	}

	InputConfigAsset = InputConfig;
	PlatformPolicyObject = PlatformPolicy;

	LoadAndApplyMappingContexts();
	BindInputActions();

	LogInputDebug(TEXT("输入路由初始化完成"));
}

void UDBAInputRouterComponent::EnableInputDebugLog(bool bEnable)
{
	bInputDebugLogEnabled = bEnable;
	LogInputDebug(FString::Printf(TEXT("输入调试日志已%s"), bEnable ? TEXT("启用") : TEXT("禁用")));
}

void UDBAInputRouterComponent::LoadAndApplyMappingContexts()
{
	if (!InputConfigAsset || !PlatformPolicyObject)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
	if (!Subsystem)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[DBAInputRouter] 无法获取 EnhancedInputLocalPlayerSubsystem"));
		return;
	}

	// 获取当前平台类型
	EDBAInputPlatform CurrentPlatform = PlatformPolicyObject->GetCurrentPlatform();
	LogInputDebug(FString::Printf(TEXT("当前平台：%d"), static_cast<uint8>(CurrentPlatform)));

	// 获取平台对应的映射上下文配置
	TArray<FDBAInputMappingContextConfig> PlatformConfigs;
	InputConfigAsset->GetMappingContextsForPlatform(CurrentPlatform, PlatformConfigs);

	// 清空现有映射
	Subsystem->ClearAllMappings();

	// 应用映射上下文
	for (const FDBAInputMappingContextConfig& Config : PlatformConfigs)
	{
		if (UInputMappingContext* MappingContext = Config.MappingContext.LoadSynchronous())
		{
			Subsystem->AddMappingContext(MappingContext, Config.Priority);
			LogInputDebug(FString::Printf(TEXT("已加载映射上下文：%s，优先级：%d"), *MappingContext->GetName(), Config.Priority));
		}
	}
}

void UDBAInputRouterComponent::BindInputActions()
{
	if (!InputConfigAsset)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[DBAInputRouter] BindInputActions 失败：Owner 不是 PlayerController"));
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[DBAInputRouter] BindInputActions 失败：InputComponent 不是 EnhancedInputComponent"));
		return;
	}

	// 绑定核心战斗输入
	const FDBACombatInputActionSet& CombatActions = InputConfigAsset->CombatActions;
	if (CombatActions.BasicAttack)
	{
		EnhancedInputComponent->BindAction(CombatActions.BasicAttack, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnBasicAttackTriggered);
	}
	if (CombatActions.Skill01)
	{
		EnhancedInputComponent->BindAction(CombatActions.Skill01, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnSkill01Triggered);
	}
	if (CombatActions.Skill02)
	{
		EnhancedInputComponent->BindAction(CombatActions.Skill02, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnSkill02Triggered);
	}
	if (CombatActions.Skill03)
	{
		EnhancedInputComponent->BindAction(CombatActions.Skill03, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnSkill03Triggered);
	}
	if (CombatActions.Skill04)
	{
		EnhancedInputComponent->BindAction(CombatActions.Skill04, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnSkill04Triggered);
	}
	if (CombatActions.Ultimate)
	{
		EnhancedInputComponent->BindAction(CombatActions.Ultimate, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnUltimateTriggered);
	}

	// 绑定系统输入
	const FDBASystemInputActionSet& SystemActions = InputConfigAsset->SystemActions;
	if (SystemActions.LockTarget)
	{
		EnhancedInputComponent->BindAction(SystemActions.LockTarget, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnLockTargetTriggered);
	}
	if (SystemActions.Ping)
	{
		EnhancedInputComponent->BindAction(SystemActions.Ping, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnPingTriggered);
	}
	if (SystemActions.Scoreboard)
	{
		EnhancedInputComponent->BindAction(SystemActions.Scoreboard, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnScoreboardTriggered);
	}
	if (SystemActions.Menu)
	{
		EnhancedInputComponent->BindAction(SystemActions.Menu, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnMenuTriggered);
	}
	if (SystemActions.Chat)
	{
		EnhancedInputComponent->BindAction(SystemActions.Chat, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnChatTriggered);
	}
	if (SystemActions.Map)
	{
		EnhancedInputComponent->BindAction(SystemActions.Map, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnMapTriggered);
	}
	if (SystemActions.Interact)
	{
		EnhancedInputComponent->BindAction(SystemActions.Interact, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnInteractTriggered);
	}

	// 绑定移动输入
	const FDBAMovementInputActionSet& MovementActions = InputConfigAsset->MovementActions;
	if (MovementActions.Move)
	{
		EnhancedInputComponent->BindAction(MovementActions.Move, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnMoveTriggered);
	}
	if (MovementActions.Look)
	{
		EnhancedInputComponent->BindAction(MovementActions.Look, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnLookTriggered);
	}
	if (MovementActions.Jump)
	{
		EnhancedInputComponent->BindAction(MovementActions.Jump, ETriggerEvent::Triggered, this, &UDBAInputRouterComponent::OnJumpTriggered);
	}

	LogInputDebug(TEXT("输入动作绑定完成"));
}

UEnhancedInputLocalPlayerSubsystem* UDBAInputRouterComponent::GetEnhancedInputSubsystem() const
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

// ========== 输入回调函数实现 ==========

void UDBAInputRouterComponent::OnBasicAttackTriggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("基础攻击输入触发"));

	// 获取 Owner Pawn 并应用输入
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return;
	}

	// 通过 PlayerController 获取 InputComponent 并应用移动
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (PC && PC->IsLocalController())
	{
		// 本地控制器直接处理输入
		// 技能激活将通过 GAS InputTag 绑定自动处理
		FVector2D MoveVector = Value.Get<FVector2D>();

		// 获取 Character 并设置移动向量
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			// UE 5.7: 使用 SetVelocityFor3DMoveVector 或直接设置 Velocity
			UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
			if (MovementComponent)
			{
				FVector MoveDir = FVector(MoveVector.X, MoveVector.Y, 0.0f);
				MovementComponent->Velocity = MoveDir.GetSafeNormal() * MovementComponent->MaxWalkSpeed;
			}
		}
	}

	// Server RPC 调用将通过 GAS 自动处理
	// 这里记录日志用于调试
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] 基础攻击触发 - 本地预测"));
	}
}

void UDBAInputRouterComponent::OnSkill01Triggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("技能01输入触发"));

	// Skill01 通过 GAS InputTag 机制自动激活
	// 此处仅记录调试日志
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] Skill01 触发"));
	}
}

void UDBAInputRouterComponent::OnSkill02Triggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("技能02输入触发"));

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] Skill02 触发"));
	}
}

void UDBAInputRouterComponent::OnSkill03Triggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("技能03输入触发"));

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] Skill03 触发"));
	}
}

void UDBAInputRouterComponent::OnSkill04Triggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("技能04输入触发"));

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] Skill04 触发"));
	}
}

void UDBAInputRouterComponent::OnUltimateTriggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("生肖大招输入触发"));

	// Ultimate 检查 UltimateEnergy 是否足够
	// GAS 系统会自动处理
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] Ultimate 触发"));
	}
}

void UDBAInputRouterComponent::OnLockTargetTriggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("锁定目标输入触发"));

	// 锁定目标切换逻辑
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		// 频率限制检查
		if (!CanSendLockTargetRPC())
		{
			UE_LOG(LogDBAUI, Verbose, TEXT("[DBAInputRouter] LockTarget 被频率限制阻止"));
			return;
		}

		// 发送 RPC 到服务端进行目标切换验证
		ServerLockTarget(nullptr);

		// 更新频率限制时间戳
		UpdateLockTargetTimestamp();

		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] 锁定目标切换"));
	}
}

void UDBAInputRouterComponent::OnPingTriggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("Ping 输入触发"));

	// Ping 系统将通过 Server RPC 处理
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		// 频率限制检查
		if (!CanSendPingRPC())
		{
			UE_LOG(LogDBAUI, Verbose, TEXT("[DBAInputRouter] Ping 被频率限制阻止"));
			return;
		}

		FVector2D PingVector = Value.Get<FVector2D>();

		// 获取 Ping 位置（世界坐标）
		FVector PingLocation = FVector(PingVector.X, PingVector.Y, 0.0f) + Pawn->GetActorLocation();

		// 发送 RPC 到服务端
		ServerPing(PingLocation);

		// 更新频率限制时间戳
		UpdatePingTimestamp();

		// 广播事件供 UI 层显示 Ping 标记
		OnPingTriggeredEvent.Broadcast(PingLocation);

		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] Ping 触发: X=%.2f, Y=%.2f, Z=%.2f"),
			PingLocation.X, PingLocation.Y, PingLocation.Z);
	}
}

void UDBAInputRouterComponent::OnScoreboardTriggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("计分板输入触发"));

	// 计分板切换
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		// 广播事件供 UI 层切换计分板显示
		OnScoreboardTriggeredEvent.Broadcast();

		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] 计分板切换"));
	}
}

void UDBAInputRouterComponent::OnMenuTriggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("菜单输入触发"));

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		// 广播事件供 UI 层打开游戏菜单
		OnMenuTriggeredEvent.Broadcast();

		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] 菜单触发"));
	}
}

void UDBAInputRouterComponent::OnChatTriggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("聊天输入触发"));

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		// 广播事件供 UI 层打开聊天输入框
		OnChatTriggeredEvent.Broadcast();

		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] 聊天触发"));
	}
}

void UDBAInputRouterComponent::OnMapTriggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("地图输入触发"));

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		// 广播事件供 UI 层切换地图显示
		OnMapTriggeredEvent.Broadcast();

		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] 地图触发"));
	}
}

void UDBAInputRouterComponent::OnInteractTriggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("交互输入触发"));

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		// 频率限制检查
		if (!CanSendInteractRPC())
		{
			UE_LOG(LogDBAUI, Verbose, TEXT("[DBAInputRouter] Interact 被频率限制阻止"));
			return;
		}

		// 通过 Actor 的 IDBAInteractableInterface 检测可交互对象
		AActor* InteractableTarget = nullptr;
		// 简单的距离检测：查找 5 米内的可交互 Actor
		TArray<AActor*> OverlappingActors;
		Pawn->GetOverlappingActors(OverlappingActors, AActor::StaticClass());
		for (AActor* Actor : OverlappingActors)
		{
			if (Actor && Actor->Implements<UDBATargetableInterface>())
			{
				if (IDBAInteractableInterface::Execute_CanInteract(Actor, Pawn))
				{
					InteractableTarget = Actor;
					break;
				}
			}
		}

		// 发送交互请求到服务端
		ServerInteract(InteractableTarget);

		// 更新频率限制时间戳
		UpdateInteractTimestamp();

		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] 交互触发: %s"),
			InteractableTarget ? *InteractableTarget->GetName() : TEXT("无目标"));
	}
}

// ========== 网络安全 RPC 实现 (WithValidation) ==========

void UDBAInputRouterComponent::ServerPing_Implementation(FVector_NetQuantize10 Location)
{
	// 服务端处理 Ping 请求
	UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] ServerPing - Location: %s"), *Location.ToString());

	// 服务端广播 Ping 到队友（通过 Multicast 或各自 Client RPC）
	// 此处记录日志，实际广播由 NetDriver 处理
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		// Ping 已收到，服务端继续处理
		UE_LOG(LogDBAUI, Verbose, TEXT("[DBAInputRouter] Ping 已路由到服务端"));
	}
}

bool UDBAInputRouterComponent::ServerPing_Validate(FVector_NetQuantize10 Location)
{
	// 验证 Ping 请求
	// 检查 Ping 位置是否在合理范围内（防止作弊发送极远位置）
	const float MaxPingDistance = 50000.0f; // 500米
	FVector OwnerLocation = FVector::ZeroVector;

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		OwnerLocation = Pawn->GetActorLocation();
	}

	float Distance = FVector::Dist(OwnerLocation, Location);
	if (Distance > MaxPingDistance)
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[DBAInputRouter] ServerPing_Validate 失败 - 距离过远: %.1f"), Distance);
		return false;
	}

	return true;
}

void UDBAInputRouterComponent::ServerLockTarget_Implementation(AActor* TargetActor)
{
	// 服务端处理锁定目标请求
	UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] ServerLockTarget - Target: %s"),
		TargetActor ? *TargetActor->GetName() : TEXT("None"));

	// 广播锁定目标变更事件
	OnLockTargetChangedEvent.Broadcast(TargetActor);
}

bool UDBAInputRouterComponent::ServerLockTarget_Validate(AActor* TargetActor)
{
	// 验证锁定目标请求
	if (!TargetActor)
	{
		// 空目标视为解除锁定，允许
		return true;
	}

	// 检查目标是否在合理范围内
	const float MaxLockTargetDistance = 3000.0f; // 30米
	FVector OwnerLocation = FVector::ZeroVector;
	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (OwnerPawn)
	{
		OwnerLocation = OwnerPawn->GetActorLocation();
	}

	float Distance = FVector::Dist(OwnerLocation, TargetActor->GetActorLocation());
	if (Distance > MaxLockTargetDistance)
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[DBAInputRouter] ServerLockTarget_Validate 失败 - 目标距离过远: %.1f"), Distance);
		return false;
	}

	// 检查目标是否可见（视线检测）
	if (!HasLineOfSightToTarget(OwnerPawn, TargetActor))
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[DBAInputRouter] ServerLockTarget_Validate 失败 - 目标不可见"));
		return false;
	}

	return true;
}

void UDBAInputRouterComponent::ServerInteract_Implementation(AActor* InteractableActor)
{
	// 服务端处理交互请求
	UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] ServerInteract - Actor: %s"),
		InteractableActor ? *InteractableActor->GetName() : TEXT("None"));

	// 通过 IDBAInteractableInterface 执行交互
	if (InteractableActor && InteractableActor->Implements<UDBATargetableInterface>())
	{
		IDBAInteractableInterface::Execute_Interact(InteractableActor, GetOwner());
	}
}

bool UDBAInputRouterComponent::ServerInteract_Validate(AActor* InteractableActor)
{
	// 验证交互请求
	if (!InteractableActor)
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[DBAInputRouter] ServerInteract_Validate 失败 - 交互对象为空"));
		return false;
	}

	// 检查交互对象是否在合理范围内
	const float MaxInteractDistance = 500.0f; // 5米
	FVector OwnerLocation = FVector::ZeroVector;

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		OwnerLocation = Pawn->GetActorLocation();
	}

	float Distance = FVector::Dist(OwnerLocation, InteractableActor->GetActorLocation());
	if (Distance > MaxInteractDistance)
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[DBAInputRouter] ServerInteract_Validate 失败 - 交互距离过远: %.1f"), Distance);
		return false;
	}

	return true;
}

void UDBAInputRouterComponent::OnMoveTriggered(const FInputActionValue& Value)
{
	FVector2D MoveVector = Value.Get<FVector2D>();
	LogInputDebug(FString::Printf(TEXT("移动输入触发：X=%.2f, Y=%.2f"), MoveVector.X, MoveVector.Y));

	// 移动输入通过 Enhanced Input 自动路由到 CharacterMovementComponent
	// 此处记录调试日志
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		// Enhanced Input 会自动处理移动输入到 PlayerController
		// 无需手动应用移动向量
	}
}

void UDBAInputRouterComponent::OnLookTriggered(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	LogInputDebug(FString::Printf(TEXT("摄像机旋转输入触发：X=%.2f, Y=%.2f"), LookVector.X, LookVector.Y));

	// 摄像机旋转通过 Enhanced Input 自动路由到 PlayerController
	// 此处记录调试日志
}

void UDBAInputRouterComponent::OnJumpTriggered(const FInputActionValue& Value)
{
	LogInputDebug(TEXT("跳跃输入触发"));

	// 跳跃通过 Enhanced Input 自动路由到 Character
	// 此处记录调试日志
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->IsLocallyControlled())
	{
		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] 跳跃触发"));
	}
}

void UDBAInputRouterComponent::LogInputDebug(const FString& Message) const
{
	if (bInputDebugLogEnabled)
	{
		UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] %s"), *Message);
	}
}

bool UDBAInputRouterComponent::CanSendPingRPC()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	float CurrentTime = World->GetTimeSeconds();
	return (CurrentTime - LastPingTime) >= MinPingInterval;
}

bool UDBAInputRouterComponent::CanSendLockTargetRPC()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	float CurrentTime = World->GetTimeSeconds();
	return (CurrentTime - LastLockTargetTime) >= MinLockTargetInterval;
}

bool UDBAInputRouterComponent::CanSendInteractRPC()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	float CurrentTime = World->GetTimeSeconds();
	return (CurrentTime - LastInteractTime) >= MinInteractInterval;
}

void UDBAInputRouterComponent::UpdatePingTimestamp()
{
	if (UWorld* World = GetWorld())
	{
		LastPingTime = World->GetTimeSeconds();
	}
}

void UDBAInputRouterComponent::UpdateLockTargetTimestamp()
{
	if (UWorld* World = GetWorld())
	{
		LastLockTargetTime = World->GetTimeSeconds();
	}
}

void UDBAInputRouterComponent::UpdateInteractTimestamp()
{
	if (UWorld* World = GetWorld())
	{
		LastInteractTime = World->GetTimeSeconds();
	}
}

bool UDBAInputRouterComponent::HasLineOfSightToTarget(APawn* SourcePawn, AActor* TargetActor) const
{
	if (!SourcePawn || !TargetActor)
	{
		return false;
	}

	// 使用 LineTrace 进行视线检测
	FVector StartLocation = SourcePawn->GetActorLocation();
	// 视线起点稍微抬高，模拟眼睛位置
	StartLocation.Z += 50.0f;

	FVector TargetLocation = TargetActor->GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(SourcePawn);
	QueryParams.AddIgnoredActor(TargetActor);

	// 使用 ECC_Camera 通道进行视线检测
	bool bHasLineOfSight = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		TargetLocation,
		ECC_Camera,
		QueryParams
	);

	if (!bHasLineOfSight)
	{
		UE_LOG(LogDBAUI, Verbose, TEXT("[DBAInputRouter] HasLineOfSightToTarget - 视线检测失败"));
	}

	return bHasLineOfSight;
}
