// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/AudioComponent.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/DBAUIFontUtils.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	const FLinearColor GoldText(0.95f, 0.77f, 0.36f, 1.0f);
	const FLinearColor DeepJade(0.015f, 0.11f, 0.085f, 0.92f);
	const FLinearColor SoftJade(0.05f, 0.28f, 0.20f, 0.78f);

	template<typename AssetType>
	AssetType* LoadAssetIfCookedAvailable(const TCHAR* ObjectPath)
	{
		if (!ObjectPath)
		{
			return nullptr;
		}

		const FSoftObjectPath SoftPath(ObjectPath);
		const FString PackageName = SoftPath.GetLongPackageName();
		if (PackageName.IsEmpty() || !FPackageName::DoesPackageExist(PackageName))
		{
			return nullptr;
		}

		return Cast<AssetType>(SoftPath.TryLoad());
	}

	UTextBlock* MakeLoginButtonLabel(UWidgetTree* WidgetTree, const FText& Label)
	{
		UTextBlock* TextBlock = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : nullptr;
		if (TextBlock)
		{
			TextBlock->SetText(Label);
			TextBlock->SetJustification(ETextJustify::Center);
			TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.18f, 0.08f, 0.0f, 1.0f)));

			FSlateFontInfo FontInfo = TextBlock->GetFont();
			FontInfo.Size = 42;
			FontInfo.OutlineSettings.OutlineSize = 1;
			FontInfo.OutlineSettings.OutlineColor = FLinearColor(1.0f, 0.86f, 0.42f, 0.65f);
			TextBlock->SetFont(FontInfo);
		}
		return TextBlock;
	}

	UTextBlock* MakeText(UWidgetTree* WidgetTree, const FName Name, const FText& Text, float Size, const FLinearColor& Color, ETextJustify::Type Justification = ETextJustify::Center)
	{
		UTextBlock* TextBlock = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name) : nullptr;
		if (TextBlock)
		{
			TextBlock->SetText(Text);
			TextBlock->SetJustification(Justification);
			TextBlock->SetColorAndOpacity(FSlateColor(Color));

			FSlateFontInfo FontInfo = TextBlock->GetFont();
			FontInfo.Size = FMath::RoundToInt(Size);
			FontInfo.OutlineSettings.OutlineSize = Size >= 36.0f ? 2 : 0;
			FontInfo.OutlineSettings.OutlineColor = FLinearColor(0.05f, 0.025f, 0.0f, 0.82f);
			TextBlock->SetFont(FontInfo);
		}
		return TextBlock;
	}

	void AddCanvasChild(UCanvasPanel* Canvas, UWidget* Child, const FVector2D& Position, const FVector2D& Size, const FVector2D& Alignment = FVector2D::ZeroVector)
	{
		if (!Canvas || !Child)
		{
			return;
		}

		if (UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child))
		{
			Slot->SetAutoSize(false);
			Slot->SetPosition(Position);
			Slot->SetSize(Size);
			Slot->SetAlignment(Alignment);
		}
	}

	void AddCanvasChildAnchored(UCanvasPanel* Canvas, UWidget* Child, const FAnchors& Anchors, const FMargin& Offsets, const FVector2D& Alignment = FVector2D::ZeroVector)
	{
		if (!Canvas || !Child)
		{
			return;
		}

		if (UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child))
		{
			Slot->SetAnchors(Anchors);
			Slot->SetOffsets(Offsets);
			Slot->SetAlignment(Alignment);
		}
	}

	FSlateBrush MakeFullBleedTextureBrush(UTexture2D* Texture)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(Texture);
		Brush.ImageSize = FVector2D(1.0f, 1.0f);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.Margin = FMargin(0.0f);
		Brush.TintColor = FSlateColor(FLinearColor::White);
		return Brush;
	}

	void StretchWidgetToParentEdges(UWidget* Widget, const int32 CanvasZOrder = -2000)
	{
		if (!Widget)
		{
			return;
		}

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			CanvasSlot->SetOffsets(FMargin(0.0f));
			CanvasSlot->SetAlignment(FVector2D::ZeroVector);
			CanvasSlot->SetZOrder(CanvasZOrder);
			return;
		}

		if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Widget->Slot))
		{
			OverlaySlot->SetPadding(FMargin(0.0f));
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	void ApplyFullBleedTextureToImage(UImage* Image, UTexture2D* Texture)
	{
		if (!Image || !Texture)
		{
			return;
		}

		Image->SetBrush(MakeFullBleedTextureBrush(Texture));
		Image->SetDesiredSizeOverride(FVector2D::ZeroVector);
		Image->SetColorAndOpacity(FLinearColor::White);
		Image->SetVisibility(ESlateVisibility::Visible);
		StretchWidgetToParentEdges(Image);
	}

	void ApplyFullBleedTextureToBorder(UBorder* Border, UTexture2D* Texture)
	{
		if (!Border || !Texture)
		{
			return;
		}

		Border->SetBrush(MakeFullBleedTextureBrush(Texture));
		Border->SetBrushColor(FLinearColor::White);
		Border->SetPadding(FMargin(0.0f));
		Border->SetVisibility(ESlateVisibility::Visible);
		StretchWidgetToParentEdges(Border);
	}

	UWidget* FindWidgetByNames(UWidgetTree* WidgetTree, const TArray<FName>& Names)
	{
		if (!WidgetTree)
		{
			return nullptr;
		}

		for (const FName& Name : Names)
		{
			if (UWidget* Widget = WidgetTree->FindWidget(Name))
			{
				return Widget;
			}
		}

		return nullptr;
	}

	bool StringContainsAny(const FString& Source, const TArray<FString>& Keywords)
	{
		for (const FString& Keyword : Keywords)
		{
			if (!Keyword.IsEmpty() && Source.Contains(Keyword))
			{
				return true;
			}
		}
		return false;
	}

	FString ExtractTextFromWidget(UWidget* Widget)
	{
		if (!Widget)
		{
			return FString();
		}

		if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			return TextBlock->GetText().ToString();
		}

		if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
		{
			const int32 Count = Panel->GetChildrenCount();
			for (int32 Index = 0; Index < Count; ++Index)
			{
				if (UWidget* Child = Panel->GetChildAt(Index))
				{
					const FString ChildText = ExtractTextFromWidget(Child);
					if (!ChildText.IsEmpty())
					{
						return ChildText;
					}
				}
			}
		}

		return FString();
	}

	UButton* FindButtonByHeuristics(
		UWidgetTree* WidgetTree,
		const TArray<FString>& NameKeywords,
		const TArray<FString>& LabelKeywords)
	{
		if (!WidgetTree)
		{
			return nullptr;
		}

		TArray<UWidget*> AllWidgets;
		WidgetTree->GetAllWidgets(AllWidgets);
		for (UWidget* Widget : AllWidgets)
		{
			UButton* Button = Cast<UButton>(Widget);
			if (!Button)
			{
				continue;
			}

			const FString ButtonName = Button->GetName();
			if (StringContainsAny(ButtonName, NameKeywords))
			{
				return Button;
			}

			if (UWidget* Content = Button->GetContent())
			{
				const FString LabelText = ExtractTextFromWidget(Content);
				if (StringContainsAny(LabelText, LabelKeywords))
				{
					return Button;
				}
			}
		}

		return nullptr;
	}

	void AddVerticalChild(UVerticalBox* Box, UWidget* Child, const FMargin& Padding, EHorizontalAlignment HorizontalAlignment = HAlign_Fill)
	{
		if (!Box || !Child)
		{
			return;
		}

		if (UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Child))
		{
			Slot->SetPadding(Padding);
			Slot->SetHorizontalAlignment(HorizontalAlignment);
		}
	}

	UButton* MakeTextButton(UWidgetTree* WidgetTree, const FName Name, const FText& Label, float FontSize, const FLinearColor& TextColor)
	{
		UButton* Button = WidgetTree ? WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name) : nullptr;
		if (!Button)
		{
			return nullptr;
		}

		FButtonStyle Style = Button->GetStyle();
		FSlateBrush NormalBrush;
		NormalBrush.DrawAs = ESlateBrushDrawType::Box;
		NormalBrush.TintColor = FSlateColor(SoftJade);
		Style.SetNormal(NormalBrush);

		FSlateBrush HoveredBrush = NormalBrush;
		HoveredBrush.TintColor = FSlateColor(FLinearColor(0.09f, 0.42f, 0.31f, 0.88f));
		Style.SetHovered(HoveredBrush);

		FSlateBrush PressedBrush = NormalBrush;
		PressedBrush.TintColor = FSlateColor(FLinearColor(0.02f, 0.16f, 0.12f, 0.96f));
		Style.SetPressed(PressedBrush);
		Button->SetStyle(Style);

		Button->AddChild(MakeText(WidgetTree, NAME_None, Label, FontSize, TextColor));
		return Button;
	}

	void ApplyEditableBoxStyle(UEditableTextBox* TextBox)
	{
		if (!TextBox)
		{
			return;
		}
		TextBox->SetMinDesiredWidth(500.0f);
		TextBox->SetVisibility(ESlateVisibility::Visible);
	}

	UWidget* FindLoginPanelWidget(UWidgetTree* WidgetTree)
	{
		if (!WidgetTree)
		{
			return nullptr;
		}

		const TArray<FName> CandidateNames = {
			TEXT("LoginPanel"),
			TEXT("LoginFormPanel"),
			TEXT("AuthPanel"),
			TEXT("Panel_Login"),
			TEXT("LoginCard")
		};

		for (const FName CandidateName : CandidateNames)
		{
			if (UWidget* Candidate = WidgetTree->FindWidget(CandidateName))
			{
				return Candidate;
			}
		}

		return nullptr;
	}

	FText GetLoginFlowStatusText(EDBALoginFlowState State)
	{
		switch (State)
		{
		case EDBALoginFlowState::TryAutoLogin:
			return NSLOCTEXT("DBALoginFlowWidget", "SigningIn", "Signing in...");
		case EDBALoginFlowState::LoadCharacterList:
			return NSLOCTEXT("DBALoginFlowWidget", "LoadingCharacters", "正在加载角色列表...");
		case EDBALoginFlowState::CharacterSelect:
			return NSLOCTEXT("DBALoginFlowWidget", "ChooseCharacter", "请选择角色。");
		case EDBALoginFlowState::CharacterCreate:
			return NSLOCTEXT("DBALoginFlowWidget", "CreateCharacter", "请创建角色。");
		case EDBALoginFlowState::MainLobby:
			return NSLOCTEXT("DBALoginFlowWidget", "EnteringLobby", "正在进入大厅...");
		default:
			return FText::GetEmpty();
		}
	}
}

UDBALoginFlowWidgetBase::UDBALoginFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FDBALoginVisualLayoutSpec UDBALoginFlowWidgetBase::GetReferenceVisualLayoutSpec()
{
	FDBALoginVisualLayoutSpec Spec;
	Spec.PanelAnchorX = 0.50f;
	Spec.TitleText = FText::FromString(TEXT("\u795E\u517D\u7ADE\u6280\u573A"));
	Spec.PrimaryButtonText = FText::FromString(TEXT("\u767B\u5F55"));
	Spec.LeftToolLabels = {
		FText::FromString(TEXT("\u516C\u544A")),
		FText::FromString(TEXT("\u5BA2\u670D")),
		FText::FromString(TEXT("\u4FEE\u590D"))
	};
	return Spec;
}

void UDBALoginFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBALoginFlowWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeVisualAssets();
	const bool bHasBlueprintLayout = WidgetTree && WidgetTree->RootWidget != nullptr;
	if (bHasBlueprintLayout)
	{
		LoginPanel = FindLoginPanelWidget(WidgetTree);
		EmailInput = Cast<UEditableTextBox>(FindWidgetByNames(WidgetTree, { TEXT("EmailInput") }));
		PasswordInput = Cast<UEditableTextBox>(FindWidgetByNames(WidgetTree, { TEXT("PasswordInput") }));
		LoginButton = Cast<UButton>(FindWidgetByNames(WidgetTree, {
			TEXT("LoginButton"),
			TEXT("BtnLogin"),
			TEXT("LoginBtn"),
			TEXT("Button_Login"),
			TEXT("PrimaryLoginButton")
		}));
		GuestLoginButton = Cast<UButton>(FindWidgetByNames(WidgetTree, {
			TEXT("GuestLoginButton"),
			TEXT("GuestButton"),
			TEXT("BtnGuestLogin"),
			TEXT("Button_GuestLogin"),
			TEXT("VisitorLoginButton"),
			TEXT("TouristLoginButton")
		}));
		DebugLoginButton = Cast<UButton>(FindWidgetByNames(WidgetTree, {
			TEXT("DebugLoginButton"),
			TEXT("BtnDebugLogin"),
			TEXT("Button_DebugLogin")
		}));
		ErrorText = Cast<UTextBlock>(FindWidgetByNames(WidgetTree, { TEXT("ErrorText") }));
		StatusText = Cast<UTextBlock>(FindWidgetByNames(WidgetTree, { TEXT("StatusText") }));
		TitleText = Cast<UTextBlock>(FindWidgetByNames(WidgetTree, { TEXT("TitleText") }));

		if (!LoginButton)
		{
			LoginButton = FindButtonByHeuristics(
				WidgetTree,
				{ TEXT("Login"), TEXT("SignIn"), TEXT("Enter") },
				{ TEXT("Login"), TEXT("Sign In"), TEXT("\u767b\u5f55"), TEXT("\u8fdb\u5165") });
		}
		if (!GuestLoginButton)
		{
			GuestLoginButton = FindButtonByHeuristics(
				WidgetTree,
				{ TEXT("Guest"), TEXT("Visitor"), TEXT("Tourist"), TEXT("Passport") },
				{ TEXT("Guest"), TEXT("Visitor"), TEXT("Tourist"), TEXT("\u6e38\u5ba2"), TEXT("\u901a\u884c\u8bc1"), TEXT("\u795e\u517d") });
		}
		if (!DebugLoginButton)
		{
			DebugLoginButton = FindButtonByHeuristics(
				WidgetTree,
				{ TEXT("Debug"), TEXT("DevLogin"), TEXT("TestLogin") },
				{ TEXT("Debug"), TEXT("Dev"), TEXT("\u8c03\u8bd5\u767b\u5f55"), TEXT("\u6d4b\u8bd5\u767b\u5f55") });
		}

		if (!EmailInput)
		{
			if (UCanvasPanel* EmailHost = Cast<UCanvasPanel>(FindWidgetByNames(WidgetTree, { TEXT("EmailInputHost") })))
			{
				EmailInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("EmailInput"));
				EmailInput->SetHintText(FText::FromString(TEXT("\u8BF7\u8F93\u5165\u8D26\u53F7")));
				ApplyEditableBoxStyle(EmailInput);
				AddCanvasChildAnchored(EmailHost, EmailInput, FAnchors(0.0f, 0.0f, 1.0f, 1.0f), FMargin(0.0f, 0.0f, 0.0f, 0.0f));
			}
		}

		if (!PasswordInput)
		{
			if (UCanvasPanel* PasswordHost = Cast<UCanvasPanel>(FindWidgetByNames(WidgetTree, { TEXT("PasswordInputHost") })))
			{
				PasswordInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("PasswordInput"));
				PasswordInput->SetHintText(FText::FromString(TEXT("\u8BF7\u8F93\u5165\u5BC6\u7801")));
				PasswordInput->SetIsPassword(true);
				ApplyEditableBoxStyle(PasswordInput);
				AddCanvasChildAnchored(PasswordHost, PasswordInput, FAnchors(0.0f, 0.0f, 1.0f, 1.0f), FMargin(0.0f, 0.0f, 0.0f, 0.0f));
			}
		}
	}
	else if (bUseReferenceNativeLayout)
	{
		BuildReferenceNativeLayout();
	}
	else
	{
		EnsureNativeFallbackLayout();
	}
	BindButtonClickAudio();
	BindControls();
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 组件绑定: LoginButton=%s, GuestLoginButton=%s, DebugLoginButton=%s, EmailInput=%s, PasswordInput=%s"),
		LoginButton ? *LoginButton->GetName() : TEXT("NULL"),
		GuestLoginButton ? *GuestLoginButton->GetName() : TEXT("NULL"),
		DebugLoginButton ? *DebugLoginButton->GetName() : TEXT("NULL"),
		EmailInput ? *EmailInput->GetName() : TEXT("NULL"),
		PasswordInput ? *PasswordInput->GetName() : TEXT("NULL"));

	InitializeAudioAssets();
	ApplyVisualStyle();
	DBAUIFonts::ApplyGameFontToWidgetTree(WidgetTree);
	ClearError();

	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		if (LoginFlow->GetFlowState() == EDBALoginFlowState::Startup)
		{
			LoginFlow->StartLoginFlow();
		}

		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowStateChanged);
		LoginFlow->OnFlowStateChanged.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowStateChanged);
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowError);
		LoginFlow->OnFlowError.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowError);
		HandleFlowStateChanged(LoginFlow->GetFlowState());
	}
}

void UDBALoginFlowWidgetBase::NativeDestruct()
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowStateChanged);
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowError);
	}

	UnbindControls();
	Super::NativeDestruct();
}

void UDBALoginFlowWidgetBase::SubmitLogin()
{
	const FString Email = EmailInput ? EmailInput->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Password = PasswordInput ? PasswordInput->GetText().ToString() : FString();

	if (Email.IsEmpty() || Password.IsEmpty())
	{
		ShowError(TEXT("请输入账号和密码。"));
		return;
	}

	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		ClearError();
		SetStatus(NSLOCTEXT("DBALoginFlowWidget", "SigningIn", "登录验证中..."));
		LoginFlow->SubmitLogin(Email, Password);
	}
	else
	{
		ShowError(TEXT("登录流程不可用。"));
	}
}

void UDBALoginFlowWidgetBase::SubmitGuestLogin()
{
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] \u70b9\u51fb\u8bbf\u5ba2\u767b\u5f55"));
	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		ClearError();
		SetStatus(NSLOCTEXT("DBALoginFlowWidget", "SigningInGuest", "\u8bbf\u5ba2\u767b\u5f55\u4e2d..."));
		LoginFlow->SubmitGuestLogin();
	}
	else
	{
		ShowError(TEXT("\u767b\u5f55\u6d41\u7a0b\u4e0d\u53ef\u7528\u3002"));
	}
}

void UDBALoginFlowWidgetBase::SubmitDebugLogin(const FString& Username)
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		ClearError();
		SetStatus(NSLOCTEXT("DBALoginFlowWidget", "SigningInDebug", "\u8c03\u8bd5\u767b\u5f55\u4e2d..."));
		LoginFlow->SubmitDebugLogin(Username);
	}
	else
	{
		ShowError(TEXT("\u767b\u5f55\u6d41\u7a0b\u4e0d\u53ef\u7528\u3002"));
	}
}

void UDBALoginFlowWidgetBase::ShowError(const FString& ErrorMessage)
{
	LastErrorMessage = ErrorMessage;
	if (ErrorText)
	{
		ErrorText->SetText(FText::FromString(ErrorMessage));
		ErrorText->SetVisibility(ErrorMessage.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	BP_OnShowError(ErrorMessage);
	UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] 错误: %s"), *ErrorMessage);
}

void UDBALoginFlowWidgetBase::ClearError()
{
	LastErrorMessage.Empty();
	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
		ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	}
	BP_OnClearError();
}

void UDBALoginFlowWidgetBase::HandleLoginClicked()
{
	SubmitLogin();
}

void UDBALoginFlowWidgetBase::HandleGuestLoginClicked()
{
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] \u70b9\u51fb\u8bbf\u5ba2\u767b\u5f55\u6309\u94ae"));
	SubmitGuestLogin();
}

void UDBALoginFlowWidgetBase::HandleDebugLoginClicked()
{
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] \u70b9\u51fb\u8c03\u8bd5\u767b\u5f55\u6309\u94ae"));
	SubmitDebugLogin(TEXT("dba_dev_01"));
}

void UDBALoginFlowWidgetBase::HandleFlowStateChanged(EDBALoginFlowState NewState)
{
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] \u6d41\u7a0b\u72b6\u6001\u53d8\u66f4: %d"), static_cast<int32>(NewState));
	SetStatus(GetLoginFlowStatusText(NewState));
	UpdateLoadingStateByFlow(NewState);
	if (NewState != EDBALoginFlowState::LoginScreen && NewState != EDBALoginFlowState::Error)
	{
		ClearError();
	}
	BP_OnFlowStateChanged(NewState);
}

void UDBALoginFlowWidgetBase::HandleFlowError(const FString& ErrorMessage)
{
	ShowError(ErrorMessage);
}

void UDBALoginFlowWidgetBase::EnsureNativeFallbackLayout()
{
	if (!WidgetTree || (EmailInput && PasswordInput && LoginButton && GuestLoginButton))
	{
		return;
	}

	UBorder* RootPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LoginPanel"));
	LoginPanel = RootPanel;
	WidgetTree->RootWidget = RootPanel;

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NativeLoginRoot"));
	RootPanel->SetContent(RootBox);

	UTextBlock* NativeTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NativeLoginTitle"));
	NativeTitleText->SetText(NSLOCTEXT("DBALoginFlowWidget", "Title", "Divine Beasts Arena"));
	RootBox->AddChildToVerticalBox(NativeTitleText);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	RootBox->AddChildToVerticalBox(StatusText);

	ErrorText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ErrorText"));
	ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	RootBox->AddChildToVerticalBox(ErrorText);

	EmailInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("EmailInput"));
	EmailInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "EmailHint", "账号"));
	RootBox->AddChildToVerticalBox(EmailInput);

	PasswordInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("PasswordInput"));
	PasswordInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "PasswordHint", "密码"));
	PasswordInput->SetIsPassword(true);
	RootBox->AddChildToVerticalBox(PasswordInput);

	LoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LoginButton"));
	LoginButton->AddChild(MakeLoginButtonLabel(WidgetTree, NSLOCTEXT("DBALoginFlowWidget", "LoginButton", "登录")));
	RootBox->AddChildToVerticalBox(LoginButton);

	GuestLoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("GuestLoginButton"));
	GuestLoginButton->AddChild(MakeLoginButtonLabel(WidgetTree, NSLOCTEXT("DBALoginFlowWidget", "GuestButton", "游客登录")));
	RootBox->AddChildToVerticalBox(GuestLoginButton);
	DebugLoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("DebugLoginButton"));
	DebugLoginButton->AddChild(MakeLoginButtonLabel(WidgetTree, NSLOCTEXT("DBALoginFlowWidget", "DebugButton", "开发账号")));
	RootBox->AddChildToVerticalBox(DebugLoginButton);
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 已创建原生回退登录布局。"));
}

void UDBALoginFlowWidgetBase::BindControls()
{
	if (LoginButton)
	{
		LoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleLoginClicked);
		LoginButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleLoginClicked);
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] LoginButton \u4e3a\u7a7a\uff0c\u672a\u7ed1\u5b9a\u70b9\u51fb\u4e8b\u4ef6\u3002"));
	}
	if (GuestLoginButton)
	{
		GuestLoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleGuestLoginClicked);
		GuestLoginButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleGuestLoginClicked);
		GuestLoginButton->SetIsEnabled(true);
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] GuestLoginButton \u4e3a\u7a7a\uff0c\u672a\u7ed1\u5b9a\u70b9\u51fb\u4e8b\u4ef6\u3002"));
	}
	if (DebugLoginButton)
	{
#if UE_BUILD_SHIPPING
		DebugLoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleDebugLoginClicked);
		DebugLoginButton->SetIsEnabled(false);
		DebugLoginButton->SetVisibility(ESlateVisibility::Collapsed);
#else
		DebugLoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleDebugLoginClicked);
		DebugLoginButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleDebugLoginClicked);
		DebugLoginButton->SetIsEnabled(true);
#endif
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] DebugLoginButton \u4e3a\u7a7a\uff0c\u672a\u7ed1\u5b9a\u70b9\u51fb\u4e8b\u4ef6\u3002"));
	}
}
void UDBALoginFlowWidgetBase::UnbindControls()
{
	if (LoginButton)
	{
		LoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleLoginClicked);
	}
	if (GuestLoginButton)
	{
		GuestLoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleGuestLoginClicked);
	}
	if (DebugLoginButton)
	{
		DebugLoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleDebugLoginClicked);
	}
}

void UDBALoginFlowWidgetBase::SetStatus(const FText& InStatusText)
{
	if (StatusText)
	{
		StatusText->SetText(InStatusText);
		StatusText->SetVisibility(InStatusText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

UDBALoginFlowSubsystem* UDBALoginFlowWidgetBase::GetLoginFlow() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>() : nullptr;
}

void UDBALoginFlowWidgetBase::HandleBackgroundMusicFinished()
{
	BackgroundMusicComponent = nullptr;
}

void UDBALoginFlowWidgetBase::InitializeAudioAssets()
{
	if (!ButtonClickSound)
	{
		ButtonClickSound = LoadAssetIfCookedAvailable<USoundBase>(TEXT("/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick.SFX_UI_ButtonClick"));
		if (!ButtonClickSound)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] 未找到按钮点击音效。"));
		}
	}

	BackgroundMusicSound = nullptr;
}

void UDBALoginFlowWidgetBase::StartBackgroundMusic()
{
	StopBackgroundMusic();
}

void UDBALoginFlowWidgetBase::StopBackgroundMusic()
{
	if (!BackgroundMusicComponent)
	{
		return;
	}

	BackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleBackgroundMusicFinished);
	BackgroundMusicComponent->Stop();
	BackgroundMusicComponent = nullptr;
}

void UDBALoginFlowWidgetBase::PlayButtonClickSfx() const
{
	if (ButtonClickSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ButtonClickSound, 0.85f);
	}
}

void UDBALoginFlowWidgetBase::InitializeVisualAssets()
{
	if (!LoginPanelTexture)
	{
		LoginPanelTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginPanel_StoneGold.T_DBA_LoginPanel_StoneGold"));
	}

	if (!LoginButtonTexture)
	{
		LoginButtonTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginButton_ParchmentGold.T_DBA_LoginButton_ParchmentGold"));
	}

	if (UTexture2D* CustomLoginBackground = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginBackground_Custom.T_DBA_LoginBackground_Custom")))
	{
		LoginBackgroundTexture = CustomLoginBackground;
	}
	else if (!LoginBackgroundTexture)
	{
		LoginBackgroundTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginForestSanctuary.T_DBA_LoginForestSanctuary"));
	}
}

void UDBALoginFlowWidgetBase::BuildReferenceNativeLayout()
{
	if (!WidgetTree)
	{
		return;
	}

	const FDBALoginVisualLayoutSpec Spec = GetReferenceVisualLayoutSpec();

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ReferenceLoginRoot"));
	WidgetTree->RootWidget = RootCanvas;

	UImage* BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ReferenceForestBackground"));
	if (LoginBackgroundTexture)
	{
		ApplyFullBleedTextureToImage(BackgroundImage, LoginBackgroundTexture);
	}
	else
	{
		FSlateBrush BackgroundBrush;
		BackgroundBrush.DrawAs = ESlateBrushDrawType::Box;
		BackgroundBrush.TintColor = FSlateColor(FLinearColor(0.0f, 0.11f, 0.09f, 1.0f));
		BackgroundImage->SetBrush(BackgroundBrush);
		BackgroundImage->SetColorAndOpacity(FLinearColor(0.0f, 0.11f, 0.09f, 1.0f));
	}
	AddCanvasChildAnchored(
		RootCanvas,
		BackgroundImage,
		FAnchors(0.0f, 0.0f, 1.0f, 1.0f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f));

	UImage* Vignette = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ReferenceVignette"));
	FSlateBrush VignetteBrush;
	VignetteBrush.DrawAs = ESlateBrushDrawType::Box;
	VignetteBrush.TintColor = FSlateColor(FLinearColor(0.0f, 0.02f, 0.015f, 0.38f));
	Vignette->SetBrush(VignetteBrush);
	Vignette->SetColorAndOpacity(FLinearColor(0.0f, 0.02f, 0.015f, 0.38f));
	AddCanvasChildAnchored(
		RootCanvas,
		Vignette,
		FAnchors(0.0f, 0.0f, 1.0f, 1.0f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f));

	UVerticalBox* LeftTools = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ReferenceLeftTools"));
	AddCanvasChild(RootCanvas, LeftTools, FVector2D(56.0f, 56.0f), FVector2D(104.0f, 330.0f));
	for (int32 Index = 0; Index < Spec.LeftToolLabels.Num(); ++Index)
	{
		const FText ButtonLabel = FText::Format(NSLOCTEXT("DBALoginFlowWidget", "LeftToolButtonFormat", "{0}\n{1}"),
			Index == 0 ? FText::FromString(TEXT("!")) : (Index == 1 ? FText::FromString(TEXT("*")) : FText::FromString(TEXT("?"))),
			Spec.LeftToolLabels[Index]);
		UButton* ToolButton = MakeTextButton(WidgetTree, FName(*FString::Printf(TEXT("ReferenceLeftTool_%d"), Index)), ButtonLabel, 24.0f, GoldText);
		AddVerticalChild(LeftTools, ToolButton, FMargin(0.0f, 0.0f, 0.0f, 24.0f));
	}

	TitleText = MakeText(WidgetTree, TEXT("ReferenceLoginTitle"), Spec.TitleText, 82.0f, FLinearColor(1.0f, 0.78f, 0.34f, 1.0f));
	AddCanvasChildAnchored(
		RootCanvas,
		TitleText,
		FAnchors(0.84f, 0.0f, 0.84f, 0.0f),
		FMargin(0.0f, 76.0f, 560.0f, 112.0f),
		FVector2D(0.5f, 0.0f));

	UTextBlock* SubtitleText = MakeText(WidgetTree, TEXT("ReferenceLoginSubtitle"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceSubtitle", "Divine Beasts Arena"), 30.0f, FLinearColor(1.0f, 0.80f, 0.45f, 1.0f));
	AddCanvasChildAnchored(
		RootCanvas,
		SubtitleText,
		FAnchors(0.84f, 0.0f, 0.84f, 0.0f),
		FMargin(0.0f, 178.0f, 540.0f, 48.0f),
		FVector2D(0.5f, 0.0f));

	UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LoginPanel"));
	LoginPanel = PanelBorder;
	AddCanvasChildAnchored(
		RootCanvas,
		PanelBorder,
		FAnchors(0.84f, 0.0f, 0.84f, 0.0f),
		FMargin(0.0f, 306.0f, 600.0f, 448.0f),
		FVector2D(0.5f, 0.0f));
	if (LoginPanelTexture)
	{
		PanelBorder->SetBrushFromTexture(LoginPanelTexture);
		PanelBorder->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.97f));
	}
	else
	{
		PanelBorder->SetBrushColor(DeepJade);
	}
	PanelBorder->SetPadding(FMargin(48.0f, 54.0f, 48.0f, 40.0f));

	UVerticalBox* FormBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ReferenceLoginForm"));
	PanelBorder->SetContent(FormBox);

	UButton* ServerButton = MakeTextButton(WidgetTree, TEXT("ReferenceServerButton"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceServer", "鎺ㄨ崘鏈嶅姟鍣?路 闈掓湪骞绘灄        v"), 25.0f, GoldText);
	AddVerticalChild(FormBox, ServerButton, FMargin(0.0f, 0.0f, 0.0f, 18.0f));

	EmailInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("EmailInput"));
	EmailInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "AccountHint", "\u8bf7\u8f93\u5165\u8d26\u53f7"));
	ApplyEditableBoxStyle(EmailInput);
	AddVerticalChild(FormBox, EmailInput, FMargin(0.0f, 0.0f, 0.0f, 16.0f));

	PasswordInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("PasswordInput"));
	PasswordInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "ReferencePasswordHint", "\u8bf7\u8f93\u5165\u5bc6\u7801"));
	PasswordInput->SetIsPassword(true);
	ApplyEditableBoxStyle(PasswordInput);
	AddVerticalChild(FormBox, PasswordInput, FMargin(0.0f, 0.0f, 0.0f, 12.0f));

	UHorizontalBox* OptionRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ReferenceLoginOptions"));
	AddVerticalChild(FormBox, OptionRow, FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	UTextBlock* RememberText = MakeText(WidgetTree, TEXT("ReferenceRememberText"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceRemember", "[x] 璁颁綇璐﹀彿"), 22.0f, GoldText, ETextJustify::Left);
	if (UHorizontalBoxSlot* RememberSlot = OptionRow->AddChildToHorizontalBox(RememberText))
	{
		RememberSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	UTextBlock* ForgotText = MakeText(WidgetTree, TEXT("ReferenceForgotText"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceForgot", "\u5fd8\u8bb0\u5bc6\u7801\uff1f"), 22.0f, GoldText, ETextJustify::Right);
	OptionRow->AddChildToHorizontalBox(ForgotText);

	LoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LoginButton"));
	LoginButton->AddChild(MakeLoginButtonLabel(WidgetTree, Spec.PrimaryButtonText));
	AddVerticalChild(FormBox, LoginButton, FMargin(0.0f), HAlign_Fill);

	ErrorText = MakeText(WidgetTree, TEXT("ErrorText"), FText::GetEmpty(), 18.0f, FLinearColor(1.0f, 0.35f, 0.24f, 1.0f));
	ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	AddVerticalChild(FormBox, ErrorText, FMargin(0.0f, 10.0f, 0.0f, 0.0f));

	StatusText = MakeText(WidgetTree, TEXT("StatusText"), FText::GetEmpty(), 18.0f, FLinearColor(0.76f, 0.96f, 0.74f, 1.0f));
	StatusText->SetVisibility(ESlateVisibility::Collapsed);
	AddVerticalChild(FormBox, StatusText, FMargin(0.0f, 6.0f, 0.0f, 0.0f));

	UTextBlock* OtherLoginText = MakeText(WidgetTree, TEXT("ReferenceOtherLoginText"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceOtherLogin", "鍏朵粬鐧诲綍鏂瑰紡"), 22.0f, GoldText);
	AddCanvasChildAnchored(
		RootCanvas,
		OtherLoginText,
		FAnchors(0.84f, 0.0f, 0.84f, 0.0f),
		FMargin(0.0f, 775.0f, 380.0f, 38.0f),
		FVector2D(0.5f, 0.0f));

	UHorizontalBox* OtherLoginRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ReferenceOtherLoginRow"));
	AddCanvasChildAnchored(
		RootCanvas,
		OtherLoginRow,
		FAnchors(0.84f, 0.0f, 0.84f, 0.0f),
		FMargin(0.0f, 822.0f, 512.0f, 96.0f),
		FVector2D(0.5f, 0.0f));
	GuestLoginButton = MakeTextButton(WidgetTree, TEXT("GuestLoginButton"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceGuestLogin", "\u795e\u517d\u901a\u884c\u8bc1"), 20.0f, GoldText);
	if (UHorizontalBoxSlot* GuestSlot = OtherLoginRow->AddChildToHorizontalBox(GuestLoginButton))
	{
		GuestSlot->SetPadding(FMargin(0.0f, 0.0f, 34.0f, 0.0f));
	}
		OtherLoginRow->AddChildToHorizontalBox(MakeTextButton(WidgetTree, TEXT("ReferenceSpiritLogin"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceSpiritLogin", "??"), 20.0f, GoldText));
	OtherLoginRow->AddChildToHorizontalBox(MakeTextButton(WidgetTree, TEXT("ReferenceJadeLogin"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceJadeLogin", "????"), 20.0f, GoldText));
	DebugLoginButton = MakeTextButton(WidgetTree, TEXT("DebugLoginButton"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceDebugLogin", "????"), 20.0f, GoldText);
	if (UHorizontalBoxSlot* DebugSlot = OtherLoginRow->AddChildToHorizontalBox(DebugLoginButton))
	{
		DebugSlot->SetPadding(FMargin(34.0f, 0.0f, 0.0f, 0.0f));
	}

	UButton* SwitchAccountButton = MakeTextButton(WidgetTree, TEXT("ReferenceSwitchAccount"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceSwitchAccount", "鍒囨崲璐﹀彿"), 20.0f, GoldText);
	AddCanvasChildAnchored(
		RootCanvas,
		SwitchAccountButton,
		FAnchors(1.0f, 1.0f, 1.0f, 1.0f),
		FMargin(-220.0f, -72.0f, 190.0f, 48.0f));
}

void UDBALoginFlowWidgetBase::ApplyLoginBackgroundTexture()
{
	if (!WidgetTree || !LoginBackgroundTexture)
	{
		return;
	}

	const auto ApplyTextureToImage = [this](UImage* Image)
	{
		ApplyFullBleedTextureToImage(Image, LoginBackgroundTexture);
	};

	if (UWidget* BackgroundWidget = FindWidgetByNames(WidgetTree, {
		TEXT("ForestBackgroundImage"),
		TEXT("LoginBackgroundImage"),
		TEXT("BackgroundImage"),
		TEXT("ReferenceForestBackground")
	}))
	{
		if (UImage* BackgroundImage = Cast<UImage>(BackgroundWidget))
		{
			ApplyTextureToImage(BackgroundImage);
			return;
		}

		if (UBorder* BackgroundBorder = Cast<UBorder>(BackgroundWidget))
		{
			ApplyFullBleedTextureToBorder(BackgroundBorder, LoginBackgroundTexture);
			return;
		}
	}

	TArray<UWidget*> AllWidgets;
	WidgetTree->GetAllWidgets(AllWidgets);
	for (UWidget* Widget : AllWidgets)
	{
		UImage* Image = Cast<UImage>(Widget);
		if (!Image)
		{
			continue;
		}

		const FString WidgetName = Image->GetName();
		if (WidgetName.Contains(TEXT("Background"), ESearchCase::IgnoreCase)
			&& !WidgetName.Contains(TEXT("Vignette"), ESearchCase::IgnoreCase)
			&& !WidgetName.Contains(TEXT("Panel"), ESearchCase::IgnoreCase)
			&& !WidgetName.Contains(TEXT("Button"), ESearchCase::IgnoreCase))
		{
			ApplyTextureToImage(Image);
			return;
		}
	}

	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return;
	}

	UImage* InjectedBackground = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("RuntimeLoginBackgroundImage"));
	ApplyTextureToImage(InjectedBackground);
	if (UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(InjectedBackground))
	{
		BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackgroundSlot->SetOffsets(FMargin(0.0f));
		BackgroundSlot->SetZOrder(-2000);
	}
}

void UDBALoginFlowWidgetBase::ApplyVisualStyle()
{
	ApplyLoginBackgroundTexture();

	if (!LoginPanel)
	{
		LoginPanel = FindLoginPanelWidget(WidgetTree);
	}

	if (LoginPanelTexture && LoginPanel)
	{
		if (UBorder* PanelBorder = Cast<UBorder>(LoginPanel))
		{
			PanelBorder->SetBrushFromTexture(LoginPanelTexture);
			PanelBorder->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.96f));
		}
		else if (UImage* PanelImage = Cast<UImage>(LoginPanel))
		{
			PanelImage->SetBrushFromTexture(LoginPanelTexture);
			PanelImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.96f));
		}
	}

	ApplyButtonTextureStyle(LoginButton);
	ApplyGuestButtonStyle(GuestLoginButton);
	ApplyGuestButtonStyle(DebugLoginButton);
	ApplyEditableBoxStyle(EmailInput);
	ApplyEditableBoxStyle(PasswordInput);
}

void UDBALoginFlowWidgetBase::ApplyButtonTextureStyle(UButton* Button) const
{
	if (!Button || !LoginButtonTexture)
	{
		return;
	}

	FSlateBrush NormalBrush;
	NormalBrush.SetResourceObject(LoginButtonTexture);
	NormalBrush.ImageSize = FVector2D(512.0f, 160.0f);
	NormalBrush.DrawAs = ESlateBrushDrawType::Image;
	NormalBrush.TintColor = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));

	FSlateBrush HoveredBrush = NormalBrush;
	HoveredBrush.TintColor = FSlateColor(FLinearColor(1.18f, 1.08f, 0.82f, 1.0f));

	FSlateBrush PressedBrush = NormalBrush;
	PressedBrush.TintColor = FSlateColor(FLinearColor(0.78f, 0.65f, 0.44f, 1.0f));

	FSlateBrush DisabledBrush = NormalBrush;
	DisabledBrush.TintColor = FSlateColor(FLinearColor(0.38f, 0.34f, 0.28f, 0.85f));

	FButtonStyle Style = Button->GetStyle();
	Style.SetNormal(NormalBrush);
	Style.SetHovered(HoveredBrush);
	Style.SetPressed(PressedBrush);
	Style.SetDisabled(DisabledBrush);
	Style.SetNormalPadding(FMargin(2.0f));
	Style.SetPressedPadding(FMargin(3.0f, 4.0f, 1.0f, 0.0f));
	Button->SetStyle(Style);
}

void UDBALoginFlowWidgetBase::UpdateLoadingStateByFlow(EDBALoginFlowState NewState)
{
	const bool bIsLoading = NewState == EDBALoginFlowState::TryAutoLogin || NewState == EDBALoginFlowState::LoadCharacterList;
	if (LoginButton)
	{
		LoginButton->SetIsEnabled(!bIsLoading);
	}
	if (GuestLoginButton)
	{
		GuestLoginButton->SetIsEnabled(!bIsLoading);
	}
	if (DebugLoginButton)
	{
#if UE_BUILD_SHIPPING
		DebugLoginButton->SetIsEnabled(false);
		DebugLoginButton->SetVisibility(ESlateVisibility::Collapsed);
#else
		DebugLoginButton->SetIsEnabled(!bIsLoading);
#endif
	}
}

void UDBALoginFlowWidgetBase::ApplyGuestButtonStyle(UButton* Button) const
{
	if (!Button)
	{
		return;
	}

	FSlateBrush NormalBrush;
	NormalBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
	NormalBrush.TintColor = FSlateColor(FLinearColor(0.05f, 0.28f, 0.62f, 0.96f));

	FSlateBrush HoveredBrush = NormalBrush;
	HoveredBrush.TintColor = FSlateColor(FLinearColor(0.08f, 0.40f, 0.84f, 1.0f));

	FSlateBrush PressedBrush = NormalBrush;
	PressedBrush.TintColor = FSlateColor(FLinearColor(0.03f, 0.18f, 0.42f, 1.0f));

	FSlateBrush DisabledBrush = NormalBrush;
	DisabledBrush.TintColor = FSlateColor(FLinearColor(0.03f, 0.10f, 0.20f, 0.72f));

	FButtonStyle Style = Button->GetStyle();
	Style.SetNormal(NormalBrush);
	Style.SetHovered(HoveredBrush);
	Style.SetPressed(PressedBrush);
	Style.SetDisabled(DisabledBrush);
	Style.SetNormalPadding(FMargin(2.0f));
	Style.SetPressedPadding(FMargin(3.0f, 4.0f, 1.0f, 0.0f));
	Button->SetStyle(Style);
}
