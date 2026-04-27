// Copyright FreeboozStudio. All Rights Reserved.

#include "DBAGameInstance.h"
#include "Common/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "Frontend/Startup/DBAStartupVideoSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Common/DBALogChannels.h"

UDBAGameInstance::UDBAGameInstance()
{
}

void UDBAGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 初始化完成"));

	// 获取启动视频子系统并播放视频
	StartupVideoSubsystem = GetSubsystem<UDBAStartupVideoSubsystem>();
	if (StartupVideoSubsystem)
	{
		StartupVideoSubsystem->PlayStartupVideo();
	}
}

void UDBAGameInstance::Shutdown()
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 关闭中"));

	Super::Shutdown();
}

void UDBAGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	Super::OnWorldChanged(OldWorld, NewWorld);

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] World 切换: %s -> %s"),
		OldWorld ? *OldWorld->GetName() : TEXT("None"),
		NewWorld ? *NewWorld->GetName() : TEXT("None"));
}
