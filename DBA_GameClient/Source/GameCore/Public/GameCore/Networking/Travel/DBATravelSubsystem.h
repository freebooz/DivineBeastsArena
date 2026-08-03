#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "DBATravelSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FDBAOnClientTravelStarted);

/**
 * 通用客户端旅行入口。
 * 只执行引擎旅行，不解析后台 DTO，也不决定业务状态。
 */
UCLASS()
class GAMECORE_API UDBATravelSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	bool RequestClientTravel(const FString& TravelUrl, FString& OutErrorMessage);

	FDBAOnClientTravelStarted OnClientTravelStarted;
};
