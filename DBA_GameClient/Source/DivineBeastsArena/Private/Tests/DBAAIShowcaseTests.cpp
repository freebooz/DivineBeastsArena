// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：为 MCP 生成的 AI_Showcase 演示资产提供最小自动化回归，避免只靠人工 PIE 和记忆判断样例是否仍然完整。
- 当前覆盖：资产存在性、交互蓝图默认引用、测试地图放置状态。
- 阅读重点：先看常量路径与辅助读取函数，再看三个测试分别覆盖的资产存在性、交互蓝图默认值和测试地图放置状态。
- 修改提示：后续如果 AI_Showcase 新增关键资产或更换交互特效引用，应优先同步本文件，再更新验证文档。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Blueprint.h"
#include "Engine/Level.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Guid.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/Field.h"
#include "UObject/UnrealType.h"

namespace
{
	const TCHAR* const AIShowcaseInteractivePropClassPath = TEXT("/Game/MCP_Generated/AI_Showcase/BP_InteractiveProp.BP_InteractiveProp_C");
	const TCHAR* const AIShowcaseInteractivePropAssetPath = TEXT("/Game/MCP_Generated/AI_Showcase/BP_InteractiveProp.BP_InteractiveProp");
	const TCHAR* const AIShowcaseMainMenuClassPath = TEXT("/Game/MCP_Generated/AI_Showcase/UI/WBP_MainMenu.WBP_MainMenu_C");
	const TCHAR* const AIShowcaseGameHudClassPath = TEXT("/Game/MCP_Generated/AI_Showcase/UI/WBP_GameHUD.WBP_GameHUD_C");
	const TCHAR* const AIShowcasePromptClassPath = TEXT("/Game/MCP_Generated/AI_Showcase/UI/WBP_InteractionPrompt.WBP_InteractionPrompt_C");
	const TCHAR* const AIShowcaseGlowMaterialPath = TEXT("/Game/MCP_Generated/AI_Showcase/Materials/MI_InteractiveGlow.MI_InteractiveGlow");
	const TCHAR* const AIShowcaseBurstV2Path = TEXT("/Game/MCP_Generated/AI_Showcase/VFX/NS_InteractionBurst_V2.NS_InteractionBurst_V2");
	const TCHAR* const AIShowcaseBurstV3Path = TEXT("/Game/MCP_Generated/AI_Showcase/VFX/NS_InteractionBurst_V3.NS_InteractionBurst_V3");
	const TCHAR* const AIShowcaseSparkPath = TEXT("/Game/MCP_Generated/AI_Showcase/VFX/NSE_InteractionSpark.NSE_InteractionSpark");
	const TCHAR* const AIShowcaseMapPath = TEXT("/Game/MCP_Generated/AI_Showcase/Maps/L_AI_Showcase_Test.L_AI_Showcase_Test");

	template <typename TObjectType>
	TObjectType* LoadRequiredAsset(FAutomationTestBase& Test, const TCHAR* AssetPath, const TCHAR* AssetLabel)
	{
		TObjectType* Asset = LoadObject<TObjectType>(nullptr, AssetPath);
		Test.TestNotNull(AssetLabel, Asset);
		return Asset;
	}

	template <typename TClassType>
	TSubclassOf<TClassType> LoadRequiredClass(FAutomationTestBase& Test, const TCHAR* ClassPath, const TCHAR* ClassLabel)
	{
		UClass* LoadedClass = StaticLoadClass(TClassType::StaticClass(), nullptr, ClassPath);
		Test.TestNotNull(ClassLabel, LoadedClass);
		return LoadedClass;
	}

	template <typename TPropertyType>
	const TPropertyType* FindRequiredProperty(FAutomationTestBase& Test, UClass* OwnerClass, const TCHAR* PropertyName)
	{
		const TPropertyType* Property = FindFProperty<TPropertyType>(OwnerClass, PropertyName);
		Test.TestNotNull(PropertyName, Property);
		return Property;
	}

	template <typename TComponentType>
	TComponentType* FindRequiredTemplateByName(FAutomationTestBase& Test, UBlueprint* BlueprintAsset, const TCHAR* ComponentName)
	{
		if (!BlueprintAsset || !BlueprintAsset->SimpleConstructionScript)
		{
			Test.AddError(TEXT("蓝图缺少 SimpleConstructionScript，无法检查组件模板。"));
			return nullptr;
		}

		for (USCS_Node* Node : BlueprintAsset->SimpleConstructionScript->GetAllNodes())
		{
			if (Node && Node->GetVariableName() == FName(ComponentName))
			{
				return Cast<TComponentType>(Node->ComponentTemplate);
			}
		}

		Test.AddError(FString::Printf(TEXT("未找到组件模板：%s"), ComponentName));
		return nullptr;
	}

	UFunction* FindRequiredFunction(FAutomationTestBase& Test, UClass* OwnerClass, const TCHAR* FunctionName)
	{
		UFunction* Function = OwnerClass ? OwnerClass->FindFunctionByName(FName(FunctionName)) : nullptr;
		Test.TestNotNull(FunctionName, Function);
		return Function;
	}

	UWidgetTree* GetRequiredWidgetTree(FAutomationTestBase& Test, UClass* WidgetClass, const TCHAR* WidgetLabel)
	{
		UWidgetBlueprintGeneratedClass* GeneratedClass = Cast<UWidgetBlueprintGeneratedClass>(WidgetClass);
		Test.TestNotNull(WidgetLabel, GeneratedClass);
		UWidgetBlueprintGeneratedClass* OwningClass = GeneratedClass ? GeneratedClass->FindWidgetTreeOwningClass() : nullptr;
		UWidgetTree* WidgetTree = OwningClass ? OwningClass->GetWidgetTreeArchetype() : nullptr;
		Test.TestNotNull(WidgetLabel, WidgetTree);
		return WidgetTree;
	}

	template <typename TWidgetType>
	TWidgetType* FindRequiredWidgetByName(FAutomationTestBase& Test, UWidgetTree* WidgetTree, const TCHAR* WidgetName, const TCHAR* WidgetLabel)
	{
		UWidget* Widget = WidgetTree ? WidgetTree->FindWidget(FName(WidgetName)) : nullptr;
		Test.TestNotNull(WidgetLabel, Widget);
		TWidgetType* TypedWidget = Cast<TWidgetType>(Widget);
		Test.TestNotNull(WidgetLabel, TypedWidget);
		return TypedWidget;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAAIShowcaseAssetsExistTest,
	"DivineBeastsArena.Showcase.AIShowcase.AssetsExist",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAAIShowcaseAssetsExistTest::RunTest(const FString& Parameters)
{
	LoadRequiredClass<AActor>(*this, AIShowcaseInteractivePropClassPath, TEXT("BP_InteractiveProp 类应存在"));
	LoadRequiredClass<UUserWidget>(*this, AIShowcaseMainMenuClassPath, TEXT("WBP_MainMenu 类应存在"));
	LoadRequiredClass<UUserWidget>(*this, AIShowcaseGameHudClassPath, TEXT("WBP_GameHUD 类应存在"));
	LoadRequiredClass<UUserWidget>(*this, AIShowcasePromptClassPath, TEXT("WBP_InteractionPrompt 类应存在"));
	LoadRequiredAsset<UMaterialInterface>(*this, AIShowcaseGlowMaterialPath, TEXT("MI_InteractiveGlow 资产应存在"));
	LoadRequiredAsset<UNiagaraSystem>(*this, AIShowcaseBurstV2Path, TEXT("NS_InteractionBurst_V2 资产应存在"));
	LoadRequiredAsset<UNiagaraSystem>(*this, AIShowcaseBurstV3Path, TEXT("NS_InteractionBurst_V3 资产应存在"));
	UObject* SparkAsset = LoadRequiredAsset<UObject>(*this, AIShowcaseSparkPath, TEXT("NSE_InteractionSpark 资产应存在"));
	if (SparkAsset)
	{
		TestEqual(
			TEXT("NSE_InteractionSpark 应保持 NiagaraStatelessEmitter 类型"),
			SparkAsset->GetClass()->GetName(),
			FString(TEXT("NiagaraStatelessEmitter")));
	}
	LoadRequiredAsset<UWorld>(*this, AIShowcaseMapPath, TEXT("L_AI_Showcase_Test 地图应存在"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAAIShowcaseWidgetTreeContractTest,
	"DivineBeastsArena.Showcase.AIShowcase.WidgetTreeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAAIShowcaseWidgetTreeContractTest::RunTest(const FString& Parameters)
{
	const TSubclassOf<UUserWidget> MainMenuClass = LoadRequiredClass<UUserWidget>(*this, AIShowcaseMainMenuClassPath, TEXT("WBP_MainMenu 类应能加载用于控件树校验"));
	const TSubclassOf<UUserWidget> GameHudClass = LoadRequiredClass<UUserWidget>(*this, AIShowcaseGameHudClassPath, TEXT("WBP_GameHUD 类应能加载用于控件树校验"));
	if (!MainMenuClass || !GameHudClass)
	{
		return false;
	}

	UWidgetTree* MainMenuWidgetTree = GetRequiredWidgetTree(*this, MainMenuClass.Get(), TEXT("WBP_MainMenu 生成类应暴露控件树"));
	UWidgetTree* GameHudWidgetTree = GetRequiredWidgetTree(*this, GameHudClass.Get(), TEXT("WBP_GameHUD 生成类应暴露控件树"));
	if (!MainMenuWidgetTree || !GameHudWidgetTree)
	{
		return false;
	}

	FindRequiredWidgetByName<UTextBlock>(*this, MainMenuWidgetTree, TEXT("AIShowcaseMenu_TitleText"), TEXT("WBP_MainMenu 应暴露 AIShowcaseMenu_TitleText"));
	FindRequiredWidgetByName<UButton>(*this, MainMenuWidgetTree, TEXT("AIShowcaseMenu_StartButton"), TEXT("WBP_MainMenu 应暴露 AIShowcaseMenu_StartButton"));
	FindRequiredWidgetByName<UButton>(*this, MainMenuWidgetTree, TEXT("AIShowcaseMenu_OptionsButton"), TEXT("WBP_MainMenu 应暴露 AIShowcaseMenu_OptionsButton"));
	FindRequiredWidgetByName<UButton>(*this, MainMenuWidgetTree, TEXT("AIShowcaseMenu_QuitButton"), TEXT("WBP_MainMenu 应暴露 AIShowcaseMenu_QuitButton"));

	FindRequiredWidgetByName<UProgressBar>(*this, GameHudWidgetTree, TEXT("AIShowcaseHUD_HealthBar"), TEXT("WBP_GameHUD 应暴露 AIShowcaseHUD_HealthBar"));
	FindRequiredWidgetByName<UProgressBar>(*this, GameHudWidgetTree, TEXT("AIShowcaseHUD_EnergyBar"), TEXT("WBP_GameHUD 应暴露 AIShowcaseHUD_EnergyBar"));
	FindRequiredWidgetByName<UTextBlock>(*this, GameHudWidgetTree, TEXT("AIShowcaseHUD_ScoreText"), TEXT("WBP_GameHUD 应暴露 AIShowcaseHUD_ScoreText"));
	FindRequiredWidgetByName<UWidget>(*this, GameHudWidgetTree, TEXT("AIShowcaseHUD_MinimapRoot"), TEXT("WBP_GameHUD 应暴露 AIShowcaseHUD_MinimapRoot"));
	FindRequiredWidgetByName<UVerticalBox>(*this, GameHudWidgetTree, TEXT("AIShowcaseHUD_EventFeedBox"), TEXT("WBP_GameHUD 应暴露 AIShowcaseHUD_EventFeedBox"));
	FindRequiredWidgetByName<UButton>(*this, GameHudWidgetTree, TEXT("AIShowcaseHUD_SkillButton_0"), TEXT("WBP_GameHUD 应暴露 AIShowcaseHUD_SkillButton_0"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAAIShowcaseInteractivePropDefaultsTest,
	"DivineBeastsArena.Showcase.AIShowcase.InteractivePropDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAAIShowcaseInteractivePropDefaultsTest::RunTest(const FString& Parameters)
{
	const TSubclassOf<AActor> PropClass = LoadRequiredClass<AActor>(*this, AIShowcaseInteractivePropClassPath, TEXT("BP_InteractiveProp 类应能加载"));
	UBlueprint* PropBlueprint = LoadRequiredAsset<UBlueprint>(*this, AIShowcaseInteractivePropAssetPath, TEXT("BP_InteractiveProp 蓝图资产应能加载"));
	if (!PropClass || !PropBlueprint)
	{
		return false;
	}

	AActor* PropCDO = PropClass->GetDefaultObject<AActor>();
	TestNotNull(TEXT("BP_InteractiveProp CDO 应存在"), PropCDO);
	if (!PropCDO)
	{
		return false;
	}

	const FBoolProperty* bIsActiveProperty = FindRequiredProperty<FBoolProperty>(*this, PropClass.Get(), TEXT("bIsActive"));
	const FBoolProperty* bOnCooldownProperty = FindRequiredProperty<FBoolProperty>(*this, PropClass.Get(), TEXT("bOnCooldown"));
	const FFloatProperty* GlowIntensityProperty = FindRequiredProperty<FFloatProperty>(*this, PropClass.Get(), TEXT("GlowIntensity"));
	const FFloatProperty* CooldownProperty = FindRequiredProperty<FFloatProperty>(*this, PropClass.Get(), TEXT("Cooldown"));
	const FTextProperty* InteractionTextProperty = FindRequiredProperty<FTextProperty>(*this, PropClass.Get(), TEXT("InteractionText"));
	if (!bIsActiveProperty || !bOnCooldownProperty || !GlowIntensityProperty || !CooldownProperty || !InteractionTextProperty)
	{
		return false;
	}

	TestTrue(TEXT("交互物默认应处于激活状态"), bIsActiveProperty->GetPropertyValue_InContainer(PropCDO));
	TestFalse(TEXT("交互物初始不应处于冷却状态"), bOnCooldownProperty->GetPropertyValue_InContainer(PropCDO));
	TestEqual(TEXT("GlowIntensity 默认值应保持文档约定值"), GlowIntensityProperty->GetPropertyValue_InContainer(PropCDO), 1.0f);
	TestEqual(TEXT("Cooldown 默认值应保持文档约定值"), CooldownProperty->GetPropertyValue_InContainer(PropCDO), 1.0f);
	TestEqual(
		TEXT("InteractionText 默认值应保持当前提示文案"),
		InteractionTextProperty->GetPropertyValue_InContainer(PropCDO).ToString(),
		FString(TEXT("Press E to Interact")));

	UNiagaraComponent* NiagaraComponent = FindRequiredTemplateByName<UNiagaraComponent>(*this, PropBlueprint, TEXT("FX_Interact"));
	UAudioComponent* AudioComponent = FindRequiredTemplateByName<UAudioComponent>(*this, PropBlueprint, TEXT("AC_Interact"));
	UBoxComponent* BoxComponent = FindRequiredTemplateByName<UBoxComponent>(*this, PropBlueprint, TEXT("BX_InteractionRange"));
	UWidgetComponent* WidgetComponent = FindRequiredTemplateByName<UWidgetComponent>(*this, PropBlueprint, TEXT("UI_InteractionPrompt"));
	if (!NiagaraComponent || !AudioComponent || !BoxComponent || !WidgetComponent)
	{
		return false;
	}

	UNiagaraSystem* ExpectedBurstSystem = LoadRequiredAsset<UNiagaraSystem>(*this, AIShowcaseBurstV3Path, TEXT("NS_InteractionBurst_V3 应能加载"));
	if (!ExpectedBurstSystem)
	{
		return false;
	}

	TestEqual(TEXT("FX_Interact 默认应使用已验证的 V3 Niagara 系统"), NiagaraComponent->GetAsset(), ExpectedBurstSystem);
	TestFalse(TEXT("FX_Interact 在重叠或交互前不应自动激活"), NiagaraComponent->bAutoActivate);
	TestFalse(TEXT("AC_Interact 在交互前不应自动播放"), AudioComponent->bAutoActivate);
	TestTrue(TEXT("交互碰撞应生成重叠事件"), BoxComponent->GetGenerateOverlapEvents());

	const TSubclassOf<UUserWidget> PromptClass = LoadRequiredClass<UUserWidget>(*this, AIShowcasePromptClassPath, TEXT("提示控件类应能加载"));
	TestTrue(TEXT("提示控件组件应使用 WBP_InteractionPrompt"), WidgetComponent->GetWidgetClass() == PromptClass.Get());
	TestTrue(TEXT("提示控件在游戏中初始应隐藏"), WidgetComponent->bHiddenInGame);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAAIShowcaseInteractionContractTest,
	"DivineBeastsArena.Showcase.AIShowcase.InteractionContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAAIShowcaseInteractionContractTest::RunTest(const FString& Parameters)
{
	const TSubclassOf<AActor> PropClass = LoadRequiredClass<AActor>(*this, AIShowcaseInteractivePropClassPath, TEXT("BP_InteractiveProp 类应能加载用于交互契约"));
	if (!PropClass)
	{
		return false;
	}

	UFunction* InteractFunction = FindRequiredFunction(*this, PropClass.Get(), TEXT("Interact"));
	UFunction* ServerInteractFunction = FindRequiredFunction(*this, PropClass.Get(), TEXT("ServerInteract"));
	UFunction* ResetCooldownFunction = FindRequiredFunction(*this, PropClass.Get(), TEXT("ResetInteractionCooldown"));
	const FBoolProperty* bOnCooldownProperty = FindRequiredProperty<FBoolProperty>(*this, PropClass.Get(), TEXT("bOnCooldown"));
	if (!InteractFunction || !ServerInteractFunction || !ResetCooldownFunction || !bOnCooldownProperty)
	{
		return false;
	}

	TestTrue(TEXT("Interact 应能被自动化无参调用"), InteractFunction->NumParms == 0);
	TestTrue(TEXT("ResetInteractionCooldown 应能被自动化无参调用"), ResetCooldownFunction->NumParms == 0);

	const FString UniqueWorldName = FString::Printf(
		TEXT("DBAAIShowcaseInteractionContractTest_%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	UPackage* WorldPackage = CreatePackage(*FString::Printf(TEXT("/Temp/%s"), *UniqueWorldName));
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, FName(*UniqueWorldName), WorldPackage, false);
	TestNotNull(TEXT("交互契约测试世界应能创建"), World);
	if (!World)
	{
		return false;
	}

	AActor* PropActor = World->SpawnActor<AActor>(PropClass.Get(), FVector::ZeroVector, FRotator::ZeroRotator);
	TestNotNull(TEXT("BP_InteractiveProp 应能在临时自动化世界中生成"), PropActor);
	if (PropActor)
	{
		TestFalse(TEXT("已生成交互物初始不应处于冷却状态"), bOnCooldownProperty->GetPropertyValue_InContainer(PropActor));
		PropActor->ProcessEvent(InteractFunction, nullptr);
		TestTrue(TEXT("Interact 在临时自动化世界中调用应安全"), IsValid(PropActor));
		PropActor->ProcessEvent(ResetCooldownFunction, nullptr);
		TestTrue(TEXT("ResetInteractionCooldown 在临时自动化世界中调用应安全"), IsValid(PropActor));
	}

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAAIShowcaseMapPlacementTest,
	"DivineBeastsArena.Showcase.AIShowcase.MapPlacement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAAIShowcaseMapPlacementTest::RunTest(const FString& Parameters)
{
	UWorld* ShowcaseWorld = LoadRequiredAsset<UWorld>(*this, AIShowcaseMapPath, TEXT("AI_Showcase 测试地图应能加载"));
	const TSubclassOf<AActor> PropClass = LoadRequiredClass<AActor>(*this, AIShowcaseInteractivePropClassPath, TEXT("BP_InteractiveProp 类应能加载用于地图校验"));
	if (!ShowcaseWorld || !PropClass)
	{
		return false;
	}

	ULevel* PersistentLevel = ShowcaseWorld->PersistentLevel;
	TestNotNull(TEXT("AI_Showcase 地图应暴露持久关卡"), PersistentLevel);
	if (!PersistentLevel)
	{
		return false;
	}

	AActor* FoundInteractiveProp = nullptr;
	for (AActor* Actor : PersistentLevel->Actors)
	{
		if (Actor && Actor->GetClass() == PropClass.Get())
		{
			FoundInteractiveProp = Actor;
			break;
		}
	}

	TestNotNull(TEXT("AI_Showcase 地图应包含 BP_InteractiveProp 实例"), FoundInteractiveProp);
	if (!FoundInteractiveProp)
	{
		return false;
	}

	TestTrue(
		TEXT("AI_Showcase 交互物应保持在文档记录的位置附近"),
		FoundInteractiveProp->GetActorLocation().Equals(FVector(0.0f, 0.0f, 100.0f), 1.0f));
	return true;
}

#endif
