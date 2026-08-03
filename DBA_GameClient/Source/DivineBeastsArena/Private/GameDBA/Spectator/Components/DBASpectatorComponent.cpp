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
#include "GameDBA/Spectator/Input/DBASpectatorInputConfigDataAsset.h"
#include "GameDBA/Gameplay/Input/Configuration/DBAEnhancedInputDeveloperSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameDBA/Gameplay/Input/Components/DBAEnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

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

	// 获取所属 PlayerController
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

	// P1-4/P1-6 改造：从旧版 BindAction/BindKey 迁移到 Enhanced Input 系统，使用项目统一基类 UDBAEnhancedInputComponent
	UDBAEnhancedInputComponent* EnhancedInputComponent = Cast<UDBAEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[观战组件] 无法获取 UDBAEnhancedInputComponent，Enhanced Input 绑定失败。请确保 DefaultInput.ini 中 DefaultInputComponentClass 配置为 /Script/DivineBeastsArena.DBAEnhancedInputComponent。"));
		return;
	}

	// 从 DeveloperSettings 加载观战输入配置
	const UDBAEnhancedInputDeveloperSettings* Settings = GetDefault<UDBAEnhancedInputDeveloperSettings>();
	const UDBASpectatorInputConfigDataAsset* InputConfig = nullptr;
	if (Settings)
	{
		InputConfig = Settings->DefaultSpectatorInputConfig.Get();
	}
	if (!InputConfig)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[观战组件] 未配置观战输入配置数据资产：请在 DBA Enhanced Input 设置中配置 DefaultSpectatorInputConfig。"));
		return;
	}

	// 添加 Input Mapping Context 到本地玩家子系统
	if (APlayerController* PC = GetOwningPlayerController())
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (UInputMappingContext* IMC = Settings->DefaultSpectatorInputMappingContext.Get())
				{
					InputSubsystem->AddMappingContext(IMC, 0);
				}
				else
				{
					UE_LOG(LogDBACore, Warning, TEXT("[观战组件] 未配置观战 Input Mapping Context：请在 DBA Enhanced Input 设置中配置 DefaultSpectatorInputMappingContext。"));
				}
			}
		}
	}

	// 绑定观战 Action（Started 事件对应旧版 IE_Pressed）
	auto BindSpectatorAction = [&](const TSoftObjectPtr<UInputAction>& ActionRef, auto Callback)
	{
		if (UInputAction* Action = ActionRef.Get())
		{
			EnhancedInputComponent->BindAction(Action, ETriggerEvent::Started, this, Callback);
		}
	};

	BindSpectatorAction(InputConfig->CycleNext, &UDBASpectatorComponent::Input_CycleNext);
	BindSpectatorAction(InputConfig->CyclePrevious, &UDBASpectatorComponent::Input_CyclePrevious);
	BindSpectatorAction(InputConfig->ToggleFreeView, &UDBASpectatorComponent::Input_ToggleFreeView);
	BindSpectatorAction(InputConfig->TogglePause, &UDBASpectatorComponent::Input_TogglePause);

	// 绑定数字键 1-9（NumericSwitchActions 数组索引 0~8 对应数字键 1~9）
	auto BindNumericSwitch = [&](int32 Index, auto Callback)
	{
		if (InputConfig->NumericSwitchActions.IsValidIndex(Index) && InputConfig->NumericSwitchActions[Index].Get())
		{
			EnhancedInputComponent->BindAction(InputConfig->NumericSwitchActions[Index].Get(), ETriggerEvent::Started, this, Callback);
		}
	};

	BindNumericSwitch(0, &UDBASpectatorComponent::Input_NumericSwitch1);
	BindNumericSwitch(1, &UDBASpectatorComponent::Input_NumericSwitch2);
	BindNumericSwitch(2, &UDBASpectatorComponent::Input_NumericSwitch3);
	BindNumericSwitch(3, &UDBASpectatorComponent::Input_NumericSwitch4);
	BindNumericSwitch(4, &UDBASpectatorComponent::Input_NumericSwitch5);
	BindNumericSwitch(5, &UDBASpectatorComponent::Input_NumericSwitch6);
	BindNumericSwitch(6, &UDBASpectatorComponent::Input_NumericSwitch7);
	BindNumericSwitch(7, &UDBASpectatorComponent::Input_NumericSwitch8);
	BindNumericSwitch(8, &UDBASpectatorComponent::Input_NumericSwitch9);
}

void UDBASpectatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新视图目标数据
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

	// 连接视图
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

void UDBASpectatorComponent::Input_CycleNext(const FInputActionValue& Value)
{
	CycleNextTarget();
}

void UDBASpectatorComponent::Input_CyclePrevious(const FInputActionValue& Value)
{
	CyclePreviousTarget();
}

void UDBASpectatorComponent::Input_ToggleFreeView(const FInputActionValue& Value)
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

void UDBASpectatorComponent::Input_NumericSwitch1(const FInputActionValue& Value) { Input_NumericSwitch(0); }
void UDBASpectatorComponent::Input_NumericSwitch2(const FInputActionValue& Value) { Input_NumericSwitch(1); }
void UDBASpectatorComponent::Input_NumericSwitch3(const FInputActionValue& Value) { Input_NumericSwitch(2); }
void UDBASpectatorComponent::Input_NumericSwitch4(const FInputActionValue& Value) { Input_NumericSwitch(3); }
void UDBASpectatorComponent::Input_NumericSwitch5(const FInputActionValue& Value) { Input_NumericSwitch(4); }
void UDBASpectatorComponent::Input_NumericSwitch6(const FInputActionValue& Value) { Input_NumericSwitch(5); }
void UDBASpectatorComponent::Input_NumericSwitch7(const FInputActionValue& Value) { Input_NumericSwitch(6); }
void UDBASpectatorComponent::Input_NumericSwitch8(const FInputActionValue& Value) { Input_NumericSwitch(7); }
void UDBASpectatorComponent::Input_NumericSwitch9(const FInputActionValue& Value) { Input_NumericSwitch(8); }

void UDBASpectatorComponent::Input_NumericSwitch(int32 Index)
{
	JumpToTarget(Index);
}

void UDBASpectatorComponent::Input_TogglePause(const FInputActionValue& Value)
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



