#pragma once

#include "CoreMinimal.h"
#include "GameBackendTypes.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "DBABackendFacadeSubsystem.generated.h"

class UDBAAccountServiceBase;

/** DBA 应用层到纯传输插件的唯一前端入口。 */
UCLASS()
class DIVINEBEASTSARENA_API UDBABackendFacadeSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	bool SynchronizeAuthentication(const UDBAAccountServiceBase* AccountService, FString& OutErrorMessage);
	void AllocateVillage(const FString& CharacterId, const FDBA_GameBackendResponseDelegate& Callback);
	void GetVillageConnection(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback);
	void TrackEvent(const FString& EventName);
};
