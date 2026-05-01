// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色动画实例基类

#include "Animation/DBAZodiacAnimInstance.h"
#include "GameFramework/Pawn.h"

UDBAZodiacAnimInstance::UDBAZodiacAnimInstance()
{
}

void UDBAZodiacAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
}

void UDBAZodiacAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (OwningPawn)
	{
		// 更新移动速度
		if (APlayerController* PC = OwningPawn->GetController<APlayerController>())
		{
			if (APlayerCameraManager* CamManager = PC->PlayerCameraManager)
			{
				// 可以在这里更新相机相关的动画变量
			}
		}
	}
}
