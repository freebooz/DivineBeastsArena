// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameMoba/Framework/DBAMobaGameModeBase.h"
#include "GameFramework/OnlineReplStructs.h"
#include "UObject/ObjectKey.h"
#include "DBAGameModeBase.generated.h"

class ADBACharacterPreviewActor;
class APlayerController;
class AController;
class APawn;

UCLASS()
class DIVINEBEASTSARENA_API ADBAGameModeBase : public ADBAMobaGameModeBase
{
	GENERATED_BODY()

public:
	ADBAGameModeBase();

	static EDBAZodiac ResolveLobbyDisplayZodiac(const FString& Options, int32 JoinIndex);
	static FTransform GetLobbyDisplayTransform(int32 JoinIndex);

protected:
	virtual void BeginPlay() override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

private:
	void SpawnOrUpdateLobbyDisplayForPlayer(APlayerController* PlayerController);
	UClass* ResolveLobbyPawnClass(EDBAZodiac Zodiac) const;

private:
	int32 NextLobbyJoinIndex = 0;
	TMap<TObjectKey<APlayerController>, int32> LobbyJoinIndices;
	TMap<TObjectKey<APlayerController>, EDBAZodiac> LobbyJoinZodiacs;
	TMap<TObjectKey<APlayerController>, TWeakObjectPtr<ADBACharacterPreviewActor>> LobbyDisplayActors;
};
