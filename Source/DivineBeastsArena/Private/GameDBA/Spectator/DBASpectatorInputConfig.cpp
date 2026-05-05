// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Spectator/DBASpectatorInputConfig.h"
#include "EnhancedInputSubsystem.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UDBASpectatorInputConfig::UDBASpectatorInputConfig()
{
}

void UDBASpectatorInputConfig::ConfigureSpectatorInput(UWorld* World)
{
	if (!World)
	{
		return;
	}

	// 遍历所有玩家控制器
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		if (!Subsystem)
		{
			continue;
		}

		// 注意: 完整的 IMC 配置需要在 UE 编辑器中创建
		// 这里提供运行时配置备选方案

		// 获取或创建 Enhanced Input Component
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
		{
			// 观战输入绑定将在组件的 SetupInputComponent 中处理
			// 参见 DBASpectatorComponent::SetupInputComponent
		}
	}
}
