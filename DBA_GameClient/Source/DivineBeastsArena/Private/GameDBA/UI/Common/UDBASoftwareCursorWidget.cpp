// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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
