// Copyright Freebooz Games, Inc. All Rights Reserved.
// 游戏实例实现 - 管理游戏生命周期和子系统

#include "DBAGameInstance.h"
#include "Common/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "Kismet/GameplayStatics.h"
#include "Common/DBALogChannels.h"

// 构造函数 - 初始化游戏实例
UDBAGameInstance::UDBAGameInstance()
{
}

// Init - 游戏实例初始化
void UDBAGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 初始化完成"));
}

// Shutdown - 游戏实例关闭
void UDBAGameInstance::Shutdown()
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 关闭中"));

	Super::Shutdown();
}

// OnWorldChanged - 世界切换时的回调
void UDBAGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	Super::OnWorldChanged(OldWorld, NewWorld);

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] World 切换: %s -> %s"),
		OldWorld ? *OldWorld->GetName() : TEXT("None"),
		NewWorld ? *NewWorld->GetName() : TEXT("None"));
}