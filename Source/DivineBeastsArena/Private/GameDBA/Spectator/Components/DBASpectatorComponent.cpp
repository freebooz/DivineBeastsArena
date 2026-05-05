// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Spectator/Components/DBASpectatorComponent.h"
#include "GameDBA/Spectator/DBASpectatorManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"

UDBASpectatorComponent::UDBASpectatorComponent()
	: Super()
	, CurrentViewMode(EDBAObserverViewMode::Follow)
	, bIsConnected(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
}

void UDBASpectatorComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取所属PlayerController
	OwningPlayerController = GetOwningPlayerController();

	// 获取观战管理器
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			SpectatorManager = GameInstance->GetSubsystem<UDBASpectatorManager>();
		}
	}
}

void UDBASpectatorComponent::SetupInputComponent(UInputComponent* InputComponent)
{
	if (!InputComponent)
	{
		return;
	}

	CachedInputComponent = InputComponent;

	// Tab键: 循环切换
	InputComponent->BindAction("Spectator_CycleNext", IE_Pressed, this, &UDBASpectatorComponent::Input_CycleNext);
	InputComponent->BindAction("Spectator_CyclePrevious", IE_Pressed, this, &UDBASpectatorComponent::Input_CyclePrevious);

	// Space键: 切换自由视角
	InputComponent->BindAction("Spectator_ToggleFreeView", IE_Pressed, this, &UDBASpectatorComponent::Input_ToggleFreeView);

	// 数字键1-9: 直接跳转
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch, 0);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch, 1);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch, 2);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch, 3);
	InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch, 4);
	InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch, 5);
	InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch, 6);
	InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch, 7);
	InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch, 8);

	// P键: 暂停
	InputComponent->BindAction("Spectator_TogglePause", IE_Pressed, this, &UDBASpectatorComponent::Input_TogglePause);
}

void UDBASpectatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新视角目标数据
	if (bIsConnected && SpectatorManager.IsValid())
	{
		CurrentViewTarget = SpectatorManager->GetCurrentViewTarget(OwningPlayerController.Get());
	}
}

void UDBASpectatorComponent::JoinSpectatorMode(FString MatchID)
{
	if (!SpectatorManager.IsValid() || !OwningPlayerController.IsValid())
	{
		return;
	}

	// 连接观战
	bool bSuccess = SpectatorManager->ConnectToMatch(OwningPlayerController.Get(), MatchID, EDBAObserverControlLevel::ViewOnly);
	if (bSuccess)
	{
		bIsConnected = true;

		// 设置输入处理
		if (APlayerController* PC = OwningPlayerController.Get())
		{
			if (UInputComponent* IC = PC->InputComponent)
			{
				SetupInputComponent(IC);
			}
		}
	}
}

void UDBASpectatorComponent::LeaveSpectatorMode()
{
	if (!SpectatorManager.IsValid() || !OwningPlayerController.IsValid())
	{
		return;
	}

	SpectatorManager->DisconnectObserver(OwningPlayerController.Get());
	bIsConnected = false;
	CurrentViewTarget = FDBAObserverViewTarget();
}

void UDBASpectatorComponent::CycleNextTarget()
{
	if (!SpectatorManager.IsValid() || !OwningPlayerController.IsValid())
	{
		return;
	}

	SpectatorManager->CycleToNextTarget(OwningPlayerController.Get());
	CurrentViewTarget = SpectatorManager->GetCurrentViewTarget(OwningPlayerController.Get());
}

void UDBASpectatorComponent::CyclePreviousTarget()
{
	if (!SpectatorManager.IsValid() || !OwningPlayerController.IsValid())
	{
		return;
	}

	SpectatorManager->CycleToPreviousTarget(OwningPlayerController.Get());
	CurrentViewTarget = SpectatorManager->GetCurrentViewTarget(OwningPlayerController.Get());
}

bool UDBASpectatorComponent::JumpToTarget(int32 TargetIndex)
{
	if (!SpectatorManager.IsValid() || !OwningPlayerController.IsValid())
	{
		return false;
	}

	bool bSuccess = SpectatorManager->SetViewTargetByIndex(OwningPlayerController.Get(), TargetIndex);
	if (bSuccess)
	{
		CurrentViewTarget = SpectatorManager->GetCurrentViewTarget(OwningPlayerController.Get());
	}
	return bSuccess;
}

void UDBASpectatorComponent::SetViewMode(EDBAObserverViewMode NewViewMode)
{
	if (!SpectatorManager.IsValid() || !OwningPlayerController.IsValid())
	{
		return;
	}

	CurrentViewMode = NewViewMode;
	SpectatorManager->SetViewMode(OwningPlayerController.Get(), NewViewMode);
}

FDBAObserverViewTarget UDBASpectatorComponent::GetCurrentViewTarget() const
{
	if (!SpectatorManager.IsValid() || !OwningPlayerController.IsValid())
	{
		return FDBAObserverViewTarget();
	}

	return SpectatorManager->GetCurrentViewTarget(OwningPlayerController.Get());
}

TArray<FDBAObserverViewTarget> UDBASpectatorComponent::GetAllViewTargets() const
{
	if (!SpectatorManager.IsValid())
	{
		return TArray<FDBAObserverViewTarget>();
	}

	return SpectatorManager->GetAllViewTargets();
}

UDBASpectatorManager* UDBASpectatorComponent::GetSpectatorManager() const
{
	return SpectatorManager.Get();
}

void UDBASpectatorComponent::Input_CycleNext()
{
	CycleNextTarget();
}

void UDBASpectatorComponent::Input_CyclePrevious()
{
	CyclePreviousTarget();
}

void UDBASpectatorComponent::Input_ToggleFreeView()
{
	if (CurrentViewMode == EDBAObserverViewMode::Follow)
	{
		SetViewMode(EDBAObserverViewMode::Free);
	}
	else
	{
		SetViewMode(EDBAObserverViewMode::Follow);
	}
}

void UDBASpectatorComponent::Input_NumericSwitch(int32 Index)
{
	JumpToTarget(Index);
}

void UDBASpectatorComponent::Input_TogglePause()
{
	if (!SpectatorManager.IsValid() || !OwningPlayerController.IsValid())
	{
		return;
	}

	SpectatorManager->TogglePause(OwningPlayerController.Get());
}

APlayerController* UDBASpectatorComponent::GetOwningPlayerController() const
{
	if (AActor* Owner = GetOwner())
	{
		if (APlayerController* PC = Cast<APlayerController>(Owner))
		{
			return PC;
		}
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Pawn->GetController<APlayerController>();
		}
	}
	return nullptr;
}
