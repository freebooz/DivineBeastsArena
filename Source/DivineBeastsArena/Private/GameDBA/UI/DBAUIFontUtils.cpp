// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/DBAUIFontUtils.h"

#include "Blueprint/WidgetTree.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Fonts/CompositeFont.h"
#include "Misc/PackageName.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	const FName DBAUIFontTypefaceName(TEXT("Regular"));
	const TCHAR* DBAUIFontFacePath = TEXT("/Game/DBA/UI/Fonts/F_DBA_ZCOOL_XiaoWei.F_DBA_ZCOOL_XiaoWei");

	UObject* GetDBAUIFontFace()
	{
		static UObject* CachedFontFace = nullptr;
		if (IsValid(CachedFontFace))
		{
			return CachedFontFace;
		}

		const FSoftObjectPath FontPath(DBAUIFontFacePath);
		const FString PackageName = FontPath.GetLongPackageName();
		if (PackageName.IsEmpty() || !FPackageName::DoesPackageExist(PackageName))
		{
			return nullptr;
		}

		CachedFontFace = FontPath.TryLoad();
		if (CachedFontFace && !CachedFontFace->IsRooted())
		{
			// FFontData stores a UObject pointer inside a Slate-owned composite font.
			// Keep the imported FontFace alive so Slate never measures text through a GC-stale font.
			CachedFontFace->AddToRoot();
		}

		return CachedFontFace;
	}

	TSharedPtr<const FCompositeFont> GetDBAUIFont()
	{
		static TSharedPtr<const FCompositeFont> CachedFont;
		if (CachedFont.IsValid())
		{
			return CachedFont;
		}

		const UObject* FontFace = GetDBAUIFontFace();
		if (!FontFace)
		{
			return nullptr;
		}

		TSharedRef<FCompositeFont> CompositeFont = MakeShared<FCompositeFont>();
		FTypefaceEntry RegularEntry(DBAUIFontTypefaceName);
		RegularEntry.Font = FFontData(FontFace);
		CompositeFont->DefaultTypeface.Fonts.Add(RegularEntry);

		CachedFont = CompositeFont;
		return CachedFont;
	}
}

FSlateFontInfo DBAUIFonts::MakeGameFont(float Size, int32 OutlineSize)
{
	FSlateFontInfo FontInfo;
	if (TSharedPtr<const FCompositeFont> CompositeFont = GetDBAUIFont())
	{
		FontInfo = FSlateFontInfo(CompositeFont, FMath::Max(1.0f, Size), DBAUIFontTypefaceName);
	}
	else
	{
		FontInfo.Size = FMath::Max(1.0f, Size);
	}

	FontInfo.OutlineSettings = FFontOutlineSettings(FMath::Max(0, OutlineSize), FLinearColor(0.02f, 0.012f, 0.004f, 0.92f));
	return FontInfo;
}

void DBAUIFonts::ApplyGameFontToWidgetTree(UWidgetTree* WidgetTree)
{
	if (!WidgetTree)
	{
		return;
	}

	WidgetTree->ForEachWidgetAndDescendants(
		[](UWidget* Widget)
		{
			if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
			{
				const FSlateFontInfo ExistingFont = TextBlock->GetFont();
				const float Size = ExistingFont.Size > 0.0f ? ExistingFont.Size : 24.0f;
				TextBlock->SetFont(DBAUIFonts::MakeGameFont(Size, 1));
				return;
			}

			if (UEditableTextBox* EditableTextBox = Cast<UEditableTextBox>(Widget))
			{
				// UE 5.7's SEditableText is sensitive to runtime-built composite fonts during prepass.
				// Keep editable fields on their default stable font; labels and buttons still use the DBA font.
				(void)EditableTextBox;
			}
		});
}
