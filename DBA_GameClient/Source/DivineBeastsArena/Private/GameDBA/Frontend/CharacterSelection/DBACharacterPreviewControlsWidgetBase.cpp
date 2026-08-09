// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterPreviewControlsWidgetBase.h"

#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBACharacterPreviewControlsWidgetBase::Rotate(const float DeltaYawDegrees)
{
	// 鼠标、手柄和触控的具体输入映射由 WBP 配置，这里只接受归一化后的旋转意图。
	if (Controller) Controller->RotatePreview(DeltaYawDegrees);
}

void UDBACharacterPreviewControlsWidgetBase::Zoom(const float DeltaDistance)
{
	if (Controller) Controller->ZoomPreview(DeltaDistance);
}

void UDBACharacterPreviewControlsWidgetBase::ResetCamera()
{
	if (Controller) Controller->ResetPreviewCamera();
}
