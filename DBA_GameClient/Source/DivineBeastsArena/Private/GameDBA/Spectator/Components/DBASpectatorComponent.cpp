// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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

	// 鑾峰彇鎵€灞濸layerController
	OwningPlayerController = GetOwningPlayerController();

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

	// Tab閿? 寰幆鍒囨崲
	InputComponent->BindAction("Spectator_CycleNext", IE_Pressed, this, &UDBASpectatorComponent::Input_CycleNext);
	InputComponent->BindAction("Spectator_CyclePrevious", IE_Pressed, this, &UDBASpectatorComponent::Input_CyclePrevious);

	// Space閿? 鍒囨崲鑷敱瑙嗚
	InputComponent->BindAction("Spectator_ToggleFreeView", IE_Pressed, this, &UDBASpectatorComponent::Input_ToggleFreeView);

	// 鏁板瓧閿?-9: 鐩存帴璺宠浆
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch1);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch2);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch3);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch4);
	InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch5);
	InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch6);
	InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch7);
	InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch8);
	InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &UDBASpectatorComponent::Input_NumericSwitch9);

	// P閿? 鏆傚仠
	InputComponent->BindAction("Spectator_TogglePause", IE_Pressed, this, &UDBASpectatorComponent::Input_TogglePause);
}

void UDBASpectatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 鏇存柊瑙嗚鐩爣鏁版嵁
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

	// 杩炴帴瑙傛垬
	bool bSuccess = SpectatorManager->ConnectToMatch(OwningPlayerController.Get(), MatchID, EDBAObserverControlLevel::ViewOnly);
	if (bSuccess)
	{
		bIsConnected = true;

		// 璁剧疆杈撳叆澶勭悊
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

void UDBASpectatorComponent::Input_NumericSwitch1() { Input_NumericSwitch(0); }
void UDBASpectatorComponent::Input_NumericSwitch2() { Input_NumericSwitch(1); }
void UDBASpectatorComponent::Input_NumericSwitch3() { Input_NumericSwitch(2); }
void UDBASpectatorComponent::Input_NumericSwitch4() { Input_NumericSwitch(3); }
void UDBASpectatorComponent::Input_NumericSwitch5() { Input_NumericSwitch(4); }
void UDBASpectatorComponent::Input_NumericSwitch6() { Input_NumericSwitch(5); }
void UDBASpectatorComponent::Input_NumericSwitch7() { Input_NumericSwitch(6); }
void UDBASpectatorComponent::Input_NumericSwitch8() { Input_NumericSwitch(7); }
void UDBASpectatorComponent::Input_NumericSwitch9() { Input_NumericSwitch(8); }

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



