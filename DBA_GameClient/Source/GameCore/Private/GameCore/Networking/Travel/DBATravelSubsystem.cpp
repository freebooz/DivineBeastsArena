#include "GameCore/Networking/Travel/DBATravelSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameCore/Core/DBALogChannels.h"

bool UDBATravelSubsystem::RequestClientTravel(const FString& TravelUrl, FString& OutErrorMessage)
{
	OutErrorMessage.Reset();
	if (TravelUrl.TrimStartAndEnd().IsEmpty())
	{
		OutErrorMessage = TEXT("旅行地址为空。");
		return false;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		OutErrorMessage = TEXT("本地玩家控制器不可用。");
		return false;
	}

	PlayerController->ClientTravel(TravelUrl, TRAVEL_Absolute);
	OnClientTravelStarted.Broadcast();
	UE_LOG(LogDBACore, Log, TEXT("[旅行子系统] 已启动客户端旅行。"));
	return true;
}
