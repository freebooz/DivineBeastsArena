// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Common/UDBASoftwareCursorWidget.h"

#include "Engine/Texture2D.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "Widgets/Images/SImage.h"

namespace
{
	constexpr float CursorVisualSize = 48.0f;
	const TCHAR* const CursorTextureObjectPath = TEXT("/Game/DBA/UI/Common/Interaction/Textures/T_DBA_MouseCursor_15.T_DBA_MouseCursor_15");
	const TCHAR* const CursorRelativePngPath = TEXT("Content/DBA/UI/Common/Interaction/Textures/T_DBA_MouseCursor_15.png");
}

TSharedRef<SWidget> UDBASoftwareCursorWidget::RebuildWidget()
{
	ConfigureCursorBrush();
	CursorImage = SNew(SImage).Image(&CursorBrush);
	return CursorImage.ToSharedRef();
}

void UDBASoftwareCursorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ConfigureCursorBrush();
	if (CursorImage.IsValid())
	{
		CursorImage->SetImage(&CursorBrush);
	}
}

UTexture2D* UDBASoftwareCursorWidget::LoadCursorTexture()
{
	if (CursorTexture)
	{
		return CursorTexture;
	}

	CursorTexture = LoadObject<UTexture2D>(nullptr, CursorTextureObjectPath);
	if (CursorTexture)
	{
		CursorTexture->NeverStream = true;
		UE_LOG(LogDBACore, Log, TEXT("[DBASoftwareCursor] Loaded cursor texture asset: %s"), CursorTextureObjectPath);
		return CursorTexture;
	}

	const FString CursorPngPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / CursorRelativePngPath);
	CursorTexture = FImageUtils::ImportFileAsTexture2D(CursorPngPath);
	if (CursorTexture)
	{
		CursorTexture->NeverStream = true;
		CursorTexture->SRGB = true;
		UE_LOG(LogDBACore, Log, TEXT("[DBASoftwareCursor] Loaded cursor texture: %s"), *CursorPngPath);
	}
	else
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBASoftwareCursor] Failed to load cursor PNG: %s"), *CursorPngPath);
	}
	return CursorTexture;
}

void UDBASoftwareCursorWidget::ConfigureCursorBrush()
{
	UTexture2D* Texture = LoadCursorTexture();
	CursorBrush = FSlateBrush();
	CursorBrush.DrawAs = ESlateBrushDrawType::Image;
	CursorBrush.ImageSize = FVector2D(CursorVisualSize, CursorVisualSize);
	CursorBrush.TintColor = FSlateColor(FLinearColor::White);
	if (Texture)
	{
		CursorBrush.SetResourceObject(Texture);
	}
}
