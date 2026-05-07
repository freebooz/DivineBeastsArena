// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameMoba/RPC/DBARpcServer.h"

bool IDBARpcServer::ServerTryActivateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
	return true;
}

bool IDBARpcServer::ServerCancelAbility_Validate(FGameplayAbilitySpecHandle Handle)
{
	return true;
}

bool IDBARpcServer::ServerLockTarget_Validate(AActor* TargetActor)
{
	return true;
}

bool IDBARpcServer::ServerMoveTo_Validate(FVector_NetQuantize10 Location)
{
	return true;
}

bool IDBARpcServer::ServerRequestAttack_Validate()
{
	return true;
}

bool IDBARpcServer::ServerUltimateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
	return true;
}