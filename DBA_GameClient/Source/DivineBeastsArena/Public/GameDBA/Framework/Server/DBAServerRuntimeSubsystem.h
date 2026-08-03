#pragma once

#include "CoreMinimal.h"
#include "GameBackendRuntimeService.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "DBAServerRuntimeSubsystem.generated.h"

/** DBA Dedicated Server 的后台运行时编排入口。 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAServerRuntimeSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	bool ConfigureAndRegister();
	bool IsConfigured() const;
	void ValidateJoinTicket(
		const FString& PlayerId,
		const FString& CharacterId,
		const FString& JoinTicket,
		const FString& Team,
		int32 SlotIndex,
		const FDBA_GameBackendRuntimePlayerBuildSummary& BuildSummary,
		FDBA_GameBackendNativeResponseCallback Callback);
	void SendHeartbeat();
	void NotifyPlayerLeft(const FString& PlayerId);
	void NotifyMatchStarted();
	void NotifyMatchEnded();
	void NotifyMatchResults(
		const FString& IdempotencyKey,
		const FString& ResultJson,
		const TArray<FDBA_GameBackendRuntimePlayerResult>& Players);

private:
	class UDBA_GameBackendRuntimeService* ResolveRuntimeService() const;
};
