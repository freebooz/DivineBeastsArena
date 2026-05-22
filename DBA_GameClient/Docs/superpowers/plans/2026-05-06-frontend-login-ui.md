# Frontend Login UI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build and verify the first playable frontend loop: startup -> login/guest login -> character list -> character creation -> main lobby.

**Architecture:** Keep login/account flow in `GameCore` and expose it through small C++ widget controllers. Add a native frontend root widget and UI manager state routing so UMG blueprints only handle layout, animation, and button binding. Use existing `Content/UI` blueprints as the asset path and keep duplicate legacy assets untouched.

**Tech Stack:** Unreal Engine 5.7.1 source build at `E:\UnrealEngine-5.7.1-release`, UE C++, UGameInstanceSubsystem, UUserWidget/UMG, existing Blueprint assets, Dedicated Server + Editor client verification.

---

## Scope Check

This plan implements only the approved A-scope frontend loop. It does not implement settings, matchmaking, arena HUD, spectator HUD, mobile HUD, or full UI asset reorganization.

The project currently has main-module compile blockers unrelated to the login UI. Task 0 resolves only the compile blockers needed to build and run the frontend loop. If more blockers appear after each build, repeat Task 0 using the same method and commit only the minimal compile fix.

## File Structure

- Modify: `Source/DivineBeastsArena/Public/GameDBA/Core/DBAEnumsCore.h`  
  Remove or rename legacy reflected enum declarations that conflict with `GameCore/Types/DBACommonEnums.h` aliases.
- Modify: `Source/GameCore/Public/GameCore/Types/DBACommonEnums.h`  
  Keep the canonical v4 enum aliases available to GameCore and frontend code.
- Modify: `Source/DivineBeastsArena/Public/GameDBA/Combat/DBAClientPredictionComponent.h`  
  Replace invalid `struct FVector;` forward declaration with a real include.
- Modify: `Source/DivineBeastsArena/Public/GameDBA/Spectator/DBASpectatorManager.h`  
  Fix `UGameInstanceSubsystem` include.
- Modify: `Source/DivineBeastsArena/Public/GameDBA/UI/DBAGameUIManager.h`
- Modify: `Source/DivineBeastsArena/Private/GameDBA/UI/DBAGameUIManager.cpp`  
  Add frontend pages and login-flow state routing.
- Create: `Source/DivineBeastsArena/Public/GameDBA/UI/Frontend/UDBAFrontendRootWidgetBase.h`
- Create: `Source/DivineBeastsArena/Private/GameDBA/UI/Frontend/UDBAFrontendRootWidgetBase.cpp`  
  Native root widget that blueprints can extend for startup/login/character/main-lobby page switching.
- Modify: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBALoginWidgetController.h`
- Modify: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBALoginWidgetController.cpp`  
  Add state query/broadcast helpers for blueprint page binding.
- Modify: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBACharacterSelectWidgetController.h`
- Modify: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterSelectWidgetController.cpp`  
  Add selected-character and create-character navigation events.
- Modify: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.h`
- Modify: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.cpp`  
  Add default values, validation result event, and fixed-skill preview helpers.
- Test: `Source/DivineBeastsArena/Private/Tests/DBAFrontendLoginUITests.cpp`  
  Automation tests for state-to-page mapping and character-create request validation.
- Modify binary assets in Unreal Editor:
  - `Content/UI/Lobby/Startup/WBP_DBA_StartupScreen.uasset`
  - `Content/UI/Lobby/Login/WBP_DBA_Login.uasset`
  - `Content/UI/Lobby/Login/WBP_DBA_GuestLoginEntry.uasset`
  - `Content/UI/Lobby/Character/WBP_DBA_CharacterSelect.uasset`
  - `Content/UI/Lobby/Character/WBP_DBA_CharacterCreate.uasset`
  - `Content/UI/Lobby/MainLobby/WBP_DBA_MainLobby.uasset`

---

### Task 0: Minimal Compile Unblock

**Files:**
- Modify: `Source/DivineBeastsArena/Public/GameDBA/Core/DBAEnumsCore.h`
- Modify: `Source/DivineBeastsArena/Public/GameDBA/Combat/DBAClientPredictionComponent.h`
- Modify: `Source/DivineBeastsArena/Public/GameDBA/Spectator/DBASpectatorManager.h`
- Modify as discovered by first build error only: files named in compiler output

- [ ] **Step 1: Run the Server build and capture the first blocker**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaServer Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

Expected: Either build succeeds or fails with concrete compiler errors. Record the first unique error group before editing.

- [ ] **Step 2: Fix `FVector` declaration blocker**

Open `Source/DivineBeastsArena/Public/GameDBA/Combat/DBAClientPredictionComponent.h`.

Replace this line:

```cpp
struct FVector;
```

with:

```cpp
#include "Math/Vector.h"
```

Expected: generated code no longer sees an invalid local `FVector` declaration.

- [ ] **Step 3: Fix spectator subsystem include**

Open `Source/DivineBeastsArena/Public/GameDBA/Spectator/DBASpectatorManager.h`.

Replace any direct include of:

```cpp
#include "Engine/GameInstanceSubsystem.h"
```

or:

```cpp
#include "GameInstanceSubsystem.h"
```

with:

```cpp
#include "Subsystems/GameInstanceSubsystem.h"
```

Expected: `UGameInstanceSubsystem` resolves against UE 5.7.1 headers.

- [ ] **Step 4: Fix enum duplicate definitions**

Open `Source/DivineBeastsArena/Public/GameDBA/Core/DBAEnumsCore.h`.

If it declares reflected enums named `EDBAZodiacType`, `EDBAElementType`, or `EDBAFiveCampType`, replace those legacy declarations with includes and aliases from the canonical GameCore enum header:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAEnumsCore.generated.h"

using EDBAZodiacType = EDBAZodiac;
using EDBAElementType = EDBAElement;
using EDBAFiveCampType = EDBAFiveCamp;
```

If UHT rejects aliases inside a generated header, remove `DBAEnumsCore.generated.h` from this header and make it a non-reflected compatibility header:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"

using EDBAZodiacType = EDBAZodiac;
using EDBAElementType = EDBAElement;
using EDBAFiveCampType = EDBAFiveCamp;
```

Expected: `DBAEnumsCore.generated.h` no longer forward declares enums that conflict with GameCore aliases.

- [ ] **Step 5: Re-run Server build**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaServer Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

Expected: The fixed error group is gone. If a new unrelated compile blocker appears, fix only that blocker and repeat this step.

- [ ] **Step 6: Re-run Editor build**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaEditor Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

Expected: The editor target builds, or the first remaining blocker is logged and fixed with the same minimal approach.

- [ ] **Step 7: Commit compile unblock**

Run:

```powershell
git add Source\DivineBeastsArena\Public\GameDBA\Core\DBAEnumsCore.h Source\DivineBeastsArena\Public\GameDBA\Combat\DBAClientPredictionComponent.h Source\DivineBeastsArena\Public\GameDBA\Spectator\DBASpectatorManager.h
git commit -m "fix: unblock frontend ui build"
```

If additional files were required by the compiler, include only those files in the same commit.

---

### Task 1: Add Frontend Root Widget

**Files:**
- Create: `Source/DivineBeastsArena/Public/GameDBA/UI/Frontend/UDBAFrontendRootWidgetBase.h`
- Create: `Source/DivineBeastsArena/Private/GameDBA/UI/Frontend/UDBAFrontendRootWidgetBase.cpp`
- Test: `Source/DivineBeastsArena/Private/Tests/DBAFrontendLoginUITests.cpp`

- [ ] **Step 1: Add state-to-page automation test**

Create `Source/DivineBeastsArena/Private/Tests/DBAFrontendLoginUITests.cpp`:

```cpp
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GameDBA/UI/Frontend/UDBAFrontendRootWidgetBase.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFrontendLoginUIPageMappingTest,
	"DivineBeastsArena.UI.Frontend.LoginFlow.PageMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFrontendLoginUIPageMappingTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Startup maps to Startup page"),
		UDBAFrontendRootWidgetBase::GetPageForLoginFlowState(EDBALoginFlowState::Startup),
		EDBAFrontendPage::Startup);
	TestEqual(TEXT("TryAutoLogin maps to Startup page"),
		UDBAFrontendRootWidgetBase::GetPageForLoginFlowState(EDBALoginFlowState::TryAutoLogin),
		EDBAFrontendPage::Startup);
	TestEqual(TEXT("LoginScreen maps to Login page"),
		UDBAFrontendRootWidgetBase::GetPageForLoginFlowState(EDBALoginFlowState::LoginScreen),
		EDBAFrontendPage::Login);
	TestEqual(TEXT("CharacterSelect maps to CharacterSelect page"),
		UDBAFrontendRootWidgetBase::GetPageForLoginFlowState(EDBALoginFlowState::CharacterSelect),
		EDBAFrontendPage::CharacterSelect);
	TestEqual(TEXT("CharacterCreate maps to CharacterCreate page"),
		UDBAFrontendRootWidgetBase::GetPageForLoginFlowState(EDBALoginFlowState::CharacterCreate),
		EDBAFrontendPage::CharacterCreate);
	TestEqual(TEXT("MainLobby maps to MainLobby page"),
		UDBAFrontendRootWidgetBase::GetPageForLoginFlowState(EDBALoginFlowState::MainLobby),
		EDBAFrontendPage::MainLobby);
	return true;
}

#endif
```

- [ ] **Step 2: Run test to verify the new type is missing**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.UI.Frontend.LoginFlow.PageMapping; Quit" -unattended -nop4 -nosplash
```

Expected: compile fails because `UDBAFrontendRootWidgetBase.h` does not exist.

- [ ] **Step 3: Create frontend root widget header**

Create `Source/DivineBeastsArena/Public/GameDBA/UI/Frontend/UDBAFrontendRootWidgetBase.h`:

```cpp
// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBAFrontendRootWidgetBase.generated.h"

class UWidget;

UENUM(BlueprintType)
enum class EDBAFrontendPage : uint8
{
	Startup,
	Login,
	CharacterSelect,
	CharacterCreate,
	MainLobby,
	Error
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAFrontendPageChanged, EDBAFrontendPage, NewPage);

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAFrontendRootWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAFrontendRootWidgetBase(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "DBA|Frontend")
	void StartFrontendFlow();

	UFUNCTION(BlueprintCallable, Category = "DBA|Frontend")
	void ShowPage(EDBAFrontendPage NewPage);

	UFUNCTION(BlueprintPure, Category = "DBA|Frontend")
	EDBAFrontendPage GetCurrentPage() const { return CurrentPage; }

	UFUNCTION(BlueprintPure, Category = "DBA|Frontend")
	static EDBAFrontendPage GetPageForLoginFlowState(EDBALoginFlowState State);

	UPROPERTY(BlueprintAssignable, Category = "DBA|Frontend")
	FDBAFrontendPageChanged OnFrontendPageChanged;

protected:
	UFUNCTION()
	void HandleLoginFlowStateChanged(EDBALoginFlowState NewState);

	UFUNCTION()
	void HandleLoginFlowError(const FString& ErrorMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Frontend", meta = (DisplayName = "On Show Page"))
	void BP_OnShowPage(EDBAFrontendPage NewPage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Frontend", meta = (DisplayName = "On Frontend Error"))
	void BP_OnFrontendError(const FString& ErrorMessage);

	UDBALoginFlowSubsystem* GetLoginFlow() const;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend")
	EDBAFrontendPage CurrentPage = EDBAFrontendPage::Startup;
};
```

- [ ] **Step 4: Create frontend root widget implementation**

Create `Source/DivineBeastsArena/Private/GameDBA/UI/Frontend/UDBAFrontendRootWidgetBase.cpp`:

```cpp
// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Frontend/UDBAFrontendRootWidgetBase.h"

UDBAFrontendRootWidgetBase::UDBAFrontendRootWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAFrontendRootWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->OnFlowStateChanged.RemoveDynamic(this, &UDBAFrontendRootWidgetBase::HandleLoginFlowStateChanged);
		Flow->OnFlowError.RemoveDynamic(this, &UDBAFrontendRootWidgetBase::HandleLoginFlowError);
		Flow->OnFlowStateChanged.AddDynamic(this, &UDBAFrontendRootWidgetBase::HandleLoginFlowStateChanged);
		Flow->OnFlowError.AddDynamic(this, &UDBAFrontendRootWidgetBase::HandleLoginFlowError);
		ShowPage(GetPageForLoginFlowState(Flow->GetFlowState()));
	}
}

void UDBAFrontendRootWidgetBase::NativeDestruct()
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->OnFlowStateChanged.RemoveDynamic(this, &UDBAFrontendRootWidgetBase::HandleLoginFlowStateChanged);
		Flow->OnFlowError.RemoveDynamic(this, &UDBAFrontendRootWidgetBase::HandleLoginFlowError);
	}

	Super::NativeDestruct();
}

void UDBAFrontendRootWidgetBase::StartFrontendFlow()
{
	ShowPage(EDBAFrontendPage::Startup);

	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->StartLoginFlow();
	}
}

void UDBAFrontendRootWidgetBase::ShowPage(EDBAFrontendPage NewPage)
{
	if (CurrentPage == NewPage)
	{
		return;
	}

	CurrentPage = NewPage;
	OnFrontendPageChanged.Broadcast(NewPage);
	BP_OnShowPage(NewPage);
}

EDBAFrontendPage UDBAFrontendRootWidgetBase::GetPageForLoginFlowState(EDBALoginFlowState State)
{
	switch (State)
	{
	case EDBALoginFlowState::Startup:
	case EDBALoginFlowState::TryAutoLogin:
		return EDBAFrontendPage::Startup;
	case EDBALoginFlowState::LoginScreen:
		return EDBAFrontendPage::Login;
	case EDBALoginFlowState::LoadCharacterList:
		return EDBAFrontendPage::Startup;
	case EDBALoginFlowState::CharacterSelect:
		return EDBAFrontendPage::CharacterSelect;
	case EDBALoginFlowState::CharacterCreate:
		return EDBAFrontendPage::CharacterCreate;
	case EDBALoginFlowState::MainLobby:
		return EDBAFrontendPage::MainLobby;
	case EDBALoginFlowState::Error:
	default:
		return EDBAFrontendPage::Error;
	}
}

void UDBAFrontendRootWidgetBase::HandleLoginFlowStateChanged(EDBALoginFlowState NewState)
{
	ShowPage(GetPageForLoginFlowState(NewState));
}

void UDBAFrontendRootWidgetBase::HandleLoginFlowError(const FString& ErrorMessage)
{
	BP_OnFrontendError(ErrorMessage);
	ShowPage(EDBAFrontendPage::Login);
}

UDBALoginFlowSubsystem* UDBAFrontendRootWidgetBase::GetLoginFlow() const
{
	const UWorld* World = GetWorld();
	return World && World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>()
		: nullptr;
}
```

- [ ] **Step 5: Run page mapping test**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.UI.Frontend.LoginFlow.PageMapping; Quit" -unattended -nop4 -nosplash
```

Expected: `DivineBeastsArena.UI.Frontend.LoginFlow.PageMapping` passes.

- [ ] **Step 6: Commit frontend root widget**

Run:

```powershell
git add Source\DivineBeastsArena\Public\GameDBA\UI\Frontend Source\DivineBeastsArena\Private\GameDBA\UI\Frontend Source\DivineBeastsArena\Private\Tests\DBAFrontendLoginUITests.cpp
git commit -m "feat: add frontend login root widget"
```

---

### Task 2: Extend UI Manager for Frontend Pages

**Files:**
- Modify: `Source/DivineBeastsArena/Public/GameDBA/UI/DBAGameUIManager.h`
- Modify: `Source/DivineBeastsArena/Private/GameDBA/UI/DBAGameUIManager.cpp`

- [ ] **Step 1: Extend UI manager header**

In `Source/DivineBeastsArena/Public/GameDBA/UI/DBAGameUIManager.h`, add the forward declaration:

```cpp
class UDBAFrontendRootWidgetBase;
```

Add `Frontend` to `EDBAUIState`:

```cpp
enum class EDBAUIState : uint8
{
	None,
	Frontend,
	MainMenu,
	Lobby,
	HeroSelect,
	Loading,
	InGame,
	Pause
};
```

Add public methods:

```cpp
UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
void ShowFrontend();

UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
void HideFrontend();
```

Add protected creator:

```cpp
UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
virtual void CreateFrontendWidget();
```

Add properties:

```cpp
UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
TObjectPtr<UDBAFrontendRootWidgetBase> FrontendWidget;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
TSubclassOf<UDBAFrontendRootWidgetBase> FrontendWidgetClass;
```

Add private flag:

```cpp
bool bFrontendVisible = false;
```

- [ ] **Step 2: Extend UI manager implementation**

In `Source/DivineBeastsArena/Private/GameDBA/UI/DBAGameUIManager.cpp`, add include:

```cpp
#include "GameDBA/UI/Frontend/UDBAFrontendRootWidgetBase.h"
```

In old-state cleanup switch, handle frontend:

```cpp
case EDBAUIState::Frontend:
	HideFrontend();
	break;
```

In new-state switch, handle frontend:

```cpp
case EDBAUIState::Frontend:
	ShowFrontend();
	break;
```

Add methods:

```cpp
void UDBAGameUIManager::ShowFrontend()
{
	if (!FrontendWidget)
	{
		CreateFrontendWidget();
	}

	if (FrontendWidget && !bFrontendVisible)
	{
		FrontendWidget->AddToViewport(0);
		FrontendWidget->Activate();
		FrontendWidget->StartFrontendFlow();
		bFrontendVisible = true;
	}
}

void UDBAGameUIManager::HideFrontend()
{
	if (FrontendWidget && bFrontendVisible)
	{
		FrontendWidget->Deactivate();
		FrontendWidget->RemoveFromParent();
		bFrontendVisible = false;
	}
}

void UDBAGameUIManager::CreateFrontendWidget()
{
	if (!FrontendWidgetClass)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			FrontendWidget = CreateWidget<UDBAFrontendRootWidgetBase>(PC, FrontendWidgetClass);
		}
	}
}
```

Update `ClearAllUI()`:

```cpp
void UDBAGameUIManager::ClearAllUI()
{
	HideFrontend();
	HideMainLobby();
	HideArenaHUD();
}
```

- [ ] **Step 3: Build Editor target**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaEditor Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

Expected: `DBAGameUIManager.cpp` compiles.

- [ ] **Step 4: Commit UI manager routing**

Run:

```powershell
git add Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp
git commit -m "feat: route frontend login pages through ui manager"
```

---

### Task 3: Improve Login and Character Widget Controllers for Blueprint Binding

**Files:**
- Modify: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBALoginWidgetController.h`
- Modify: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBALoginWidgetController.cpp`
- Modify: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBACharacterSelectWidgetController.h`
- Modify: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterSelectWidgetController.cpp`
- Modify: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.h`
- Modify: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.cpp`

- [ ] **Step 1: Add login state accessors**

In `UDBALoginWidgetController.h`, add:

```cpp
UFUNCTION(BlueprintPure, Category = "DBA|Login")
EDBALoginFlowState GetCurrentLoginState() const;

UFUNCTION(BlueprintCallable, Category = "DBA|Login")
void RebindAndBroadcastState();
```

In `UDBALoginWidgetController.cpp`, add:

```cpp
EDBALoginFlowState UDBALoginWidgetController::GetCurrentLoginState() const
{
	if (const UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		return Flow->GetFlowState();
	}
	return EDBALoginFlowState::Startup;
}

void UDBALoginWidgetController::RebindAndBroadcastState()
{
	Start();
	OnLoginStateChanged.Broadcast(GetCurrentLoginState());
}
```

- [ ] **Step 2: Add character-select navigation event**

In `UDBACharacterSelectWidgetController.h`, add:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBARequestCharacterCreate);

UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
void RequestCreateCharacter();

UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterSelect")
FDBARequestCharacterCreate OnRequestCreateCharacter;
```

In `UDBACharacterSelectWidgetController.cpp`, add:

```cpp
void UDBACharacterSelectWidgetController::RequestCreateCharacter()
{
	OnRequestCreateCharacter.Broadcast();
}
```

- [ ] **Step 3: Add character-create validation and preview helpers**

In `UDBACharacterCreateWidgetController.h`, add delegate:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBACharacterCreateValidationFailed, const FString&, ErrorMessage);
```

Add public methods:

```cpp
UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
void ResetDefaults();

UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate")
FString GetFixedSkillGroupPreview() const;

UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate")
bool CanSubmit() const;

UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterCreate")
FDBACharacterCreateValidationFailed OnValidationFailed;
```

In `UDBACharacterCreateWidgetController.cpp`, add:

```cpp
void UDBACharacterCreateWidgetController::ResetDefaults()
{
	PendingRequest.CharacterName = TEXT("水灵鼠");
	PendingRequest.Zodiac = EDBAZodiac::Rat;
	PendingRequest.PrimaryElement = EDBAElement::Water;
	PendingRequest.FiveCamp = EDBAFiveCamp::West;
	PendingRequest.DefaultZodiac = PendingRequest.Zodiac;
	PendingRequest.DefaultElement = PendingRequest.PrimaryElement;
	PendingRequest.DefaultFiveCamp = PendingRequest.FiveCamp;
}

FString UDBACharacterCreateWidgetController::GetFixedSkillGroupPreview() const
{
	return FString::Printf(TEXT("%s_%s"),
		*UEnum::GetValueAsString(PendingRequest.Zodiac).RightChop(FString(TEXT("EDBAZodiac::")).Len()),
		*UEnum::GetValueAsString(PendingRequest.PrimaryElement).RightChop(FString(TEXT("EDBAElement::")).Len()));
}

bool UDBACharacterCreateWidgetController::CanSubmit() const
{
	return !PendingRequest.CharacterName.TrimStartAndEnd().IsEmpty()
		&& PendingRequest.Zodiac != EDBAZodiac::None
		&& PendingRequest.PrimaryElement != EDBAElement::None
		&& PendingRequest.FiveCamp != EDBAFiveCamp::None;
}
```

Update `Submit()`:

```cpp
void UDBACharacterCreateWidgetController::Submit()
{
	if (!CanSubmit())
	{
		OnValidationFailed.Broadcast(TEXT("请先完成名称、生肖、元素和阵营选择"));
		return;
	}

	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->SubmitCharacterCreation(PendingRequest);
	}
}
```

- [ ] **Step 4: Build Editor target**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaEditor Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

Expected: the three login/character controllers compile.

- [ ] **Step 5: Commit controller binding helpers**

Run:

```powershell
git add Source\DivineBeastsArena\Public\GameDBA\UI\Lobby\Login Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\Login
git commit -m "feat: expose frontend login ui binding helpers"
```

---

### Task 4: Bind UMG Blueprint Layouts in the Editor

**Files:**
- Modify binary: `Content/UI/Lobby/Startup/WBP_DBA_StartupScreen.uasset`
- Modify binary: `Content/UI/Lobby/Login/WBP_DBA_Login.uasset`
- Modify binary: `Content/UI/Lobby/Login/WBP_DBA_GuestLoginEntry.uasset`
- Modify binary: `Content/UI/Lobby/Character/WBP_DBA_CharacterSelect.uasset`
- Modify binary: `Content/UI/Lobby/Character/WBP_DBA_CharacterCreate.uasset`
- Modify binary: `Content/UI/Lobby/MainLobby/WBP_DBA_MainLobby.uasset`

- [ ] **Step 1: Open the project with the required engine**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Binaries\Win64\UnrealEditor.exe' "$PWD\DivineBeastsArena.uproject" -log
```

Expected: Unreal Editor opens the project.

- [ ] **Step 2: Create or reparent the frontend root blueprint**

In the Editor:

1. Create `Content/UI/Lobby/WBP_DBA_FrontendRoot` if it does not exist.
2. Parent class: `UDBAFrontendRootWidgetBase`.
3. Add a full-screen root `CanvasPanel`.
4. Add child containers named:
   - `StartupLayer`
   - `LoginLayer`
   - `CharacterSelectLayer`
   - `CharacterCreateLayer`
   - `MainLobbyLayer`
5. Place the approved page widgets into their matching layer.
6. Implement `BP_OnShowPage` so exactly one layer is visible for each `EDBAFrontendPage`.
7. Implement `BP_OnFrontendError` to show `WBP_DBA_ErrorBanner`.

Expected: Calling `ShowPage(Login)` hides all layers except `LoginLayer`.

- [ ] **Step 3: Bind login page**

In `Content/UI/Lobby/Login/WBP_DBA_Login`:

1. Ensure parent remains compatible with `UUserWidget` or the existing project base.
2. Add text inputs named `EmailInput` and `PasswordInput`.
3. Add buttons named `LoginButton` and `GuestLoginButton`.
4. Add an error text or banner area named `ErrorBanner`.
5. On construct, create or receive `UDBALoginWidgetController`.
6. Bind `LoginButton.OnClicked` to `LoginWithEmail(EmailInput.Text, PasswordInput.Text)`.
7. Bind `GuestLoginButton.OnClicked` to `LoginAsGuest()`.
8. Bind `OnLoginError` to show `ErrorBanner`.

Expected: clicking guest login invokes `SubmitGuestLogin()` on the login flow subsystem.

- [ ] **Step 4: Bind character select page**

In `Content/UI/Lobby/Character/WBP_DBA_CharacterSelect`:

1. Add a list container named `CharacterList`.
2. Add a button named `CreateCharacterButton`.
3. On construct, create or receive `UDBACharacterSelectWidgetController`.
4. Call `BindLoginFlow()`.
5. Bind `OnCharactersChanged` to rebuild `CharacterList`.
6. Each character row must call `SelectCharacter(CharacterId)` when clicked.
7. Bind `CreateCharacterButton.OnClicked` to `RequestCreateCharacter()`.
8. Bind `OnRequestCreateCharacter` to ask the frontend root to show `CharacterCreate`.

Expected: cached characters appear without requiring a second login request.

- [ ] **Step 5: Bind character create page**

In `Content/UI/Lobby/Character/WBP_DBA_CharacterCreate`:

1. Add input named `CharacterNameInput`.
2. Add selection buttons for `Rat`, `Ox`, `Tiger`, `Rabbit`, `Dragon`, `Snake`, `Horse`, `Goat`, `Monkey`, `Rooster`, `Dog`, `Pig`.
3. Add element buttons for `Gold`, `Wood`, `Water`, `Fire`, `Earth`.
4. Add five-camp buttons for `West`, `East`, `North`, `South`, `Center`.
5. Add text block named `FixedSkillGroupPreview`.
6. Add button named `SubmitButton`.
7. On construct, create or receive `UDBACharacterCreateWidgetController` and call `ResetDefaults()`.
8. Bind name input to `SetCharacterName`.
9. Bind selection buttons to `SetZodiac`, `SetElement`, and `SetFiveCamp`.
10. Refresh `FixedSkillGroupPreview` from `GetFixedSkillGroupPreview()`.
11. Bind `SubmitButton.OnClicked` to `Submit()`.
12. Bind `OnValidationFailed` to show `WBP_DBA_ErrorBanner`.

Expected: default selection previews `Rat_Water`; submit creates a valid request.

- [ ] **Step 6: Bind main lobby page**

In `Content/UI/Lobby/MainLobby/WBP_DBA_MainLobby`:

1. Parent or keep existing parent compatible with `UDBAMainLobbyWidgetBase`.
2. Add current character summary area named `CurrentCharacterPanel`.
3. Add buttons named `StartMatchButton`, `PartyButton`, `SettingsButton`, and `LogoutButton`.
4. Bind `StartMatchButton` to a disabled-state toast saying `匹配功能将在下一阶段启用`.
5. Bind `PartyButton` to a disabled-state toast saying `组队功能将在下一阶段启用`.
6. Bind `SettingsButton` to a disabled-state toast saying `设置功能将在下一阶段启用`.
7. Bind `LogoutButton` to return to the Login page through the frontend root.

Expected: main lobby is visible after character creation and does not require matchmaking to be implemented.

- [ ] **Step 7: Save and commit blueprint assets**

In Editor, save all modified assets.

Run:

```powershell
git status --short Content\UI\Lobby
git add Content\UI\Lobby
git commit -m "feat: bind frontend login blueprint layouts"
```

Expected: only the approved frontend UI assets are staged.

---

### Task 5: Configure Startup Map and UI Entry

**Files:**
- Modify: `Config/DefaultEngine.ini`
- Modify as needed: `Content/Maps/Lobby/FrontendMap.umap` or `Content/Maps/Lobby/LobbyMap.umap`
- Modify as needed: `Content\Blueprints\GI_Main.uasset` or active GameInstance blueprint

- [ ] **Step 1: Inspect current map settings**

Run:

```powershell
rg -n "GameDefaultMap|ServerDefaultMap|GameInstanceClass|GlobalDefaultGameMode" Config
```

Expected: output identifies the current startup map and GameInstance class.

- [ ] **Step 2: Set frontend-capable startup map**

If `FrontendMap` exists and opens, set in `Config/DefaultEngine.ini`:

```ini
[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=/Game/Maps/Lobby/FrontendMap
ServerDefaultMap=/Game/Maps/Lobby/LobbyMap
```

If `FrontendMap` does not load, use:

```ini
[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=/Game/Maps/Lobby/LobbyMap
ServerDefaultMap=/Game/Maps/Lobby/LobbyMap
```

Expected: client starts on a map that can show the frontend root.

- [ ] **Step 3: Bind frontend widget class to UI manager**

In the active GameInstance or UI manager blueprint:

1. Set `FrontendWidgetClass` to `Content/UI/Lobby/WBP_DBA_FrontendRoot`.
2. Ensure startup calls `UDBAGameUIManager::TransitionTo(EDBAUIState::Frontend)`.

If there is no existing blueprint entry, add a C++ fallback in `UDBAGameUIManager::Initialize`:

```cpp
void UDBAGameUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (CurrentState == EDBAUIState::None)
	{
		TransitionTo(EDBAUIState::Frontend);
	}
}
```

Expected: frontend root appears automatically on client startup.

- [ ] **Step 4: Build Editor target**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaEditor Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

Expected: build succeeds.

- [ ] **Step 5: Commit startup configuration**

Run:

```powershell
git add Config\DefaultEngine.ini Content\Maps\Lobby Content\Blueprints\GI_Main.uasset Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp
git commit -m "feat: start client in frontend login flow"
```

Stage only files that changed.

---

### Task 6: End-to-End Dedicated Server and Client Verification

**Files:**
- Create: `Docs/superpowers/verification/2026-05-06-frontend-login-ui-run.md`

- [ ] **Step 1: Build Dedicated Server**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaServer Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

Expected: exit code `0`.

- [ ] **Step 2: Build Editor client**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaEditor Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

Expected: exit code `0`.

- [ ] **Step 3: Start Dedicated Server**

Run in a background terminal:

```powershell
Start-Process -FilePath 'E:\UnrealEngine-5.7.1-release\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' -ArgumentList @(
	"$PWD\DivineBeastsArena.uproject",
	"LobbyMap",
	"-server",
	"-log",
	"-nosteam"
) -WindowStyle Hidden
```

Expected: server process starts and stays running.

- [ ] **Step 4: Start client**

Run:

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Binaries\Win64\UnrealEditor.exe' "$PWD\DivineBeastsArena.uproject" 127.0.0.1 -game -log -nosteam
```

Expected: client opens and reaches frontend root.

- [ ] **Step 5: Verify player login to lobby**

Manual verification in client:

1. Wait for startup page.
2. Click guest login.
3. Confirm login flow reaches character list.
4. If no characters exist, create one with:
   - Name: `水灵鼠`
   - Zodiac: `Rat`
   - Element: `Water`
   - FiveCamp: `West`
5. Confirm UI enters main lobby.
6. Confirm no blocking error modal remains visible.

Expected: the player reaches `WBP_DBA_MainLobby`.

- [ ] **Step 6: Write verification report**

Create `Docs/superpowers/verification/2026-05-06-frontend-login-ui-run.md`:

```markdown
# Frontend Login UI Verification

Date: 2026-05-06
Engine: E:\UnrealEngine-5.7.1-release

## Build

- DivineBeastsArenaServer Win64 Development: PASS
- DivineBeastsArenaEditor Win64 Development: PASS

## Runtime

- Dedicated Server launched on LobbyMap: PASS
- Client launched and connected to 127.0.0.1: PASS
- Startup page visible: PASS
- Guest login or Mock fallback: PASS
- Character list loaded: PASS
- Character creation completed when empty: PASS
- Main lobby visible: PASS

## Notes

- Backend URL: http://127.0.0.1:8080
- Mock fallback used when backend was unavailable.
```

If any item fails, replace `PASS` with `FAIL` and include the exact log line or visible symptom.

- [ ] **Step 7: Commit verification report**

Run:

```powershell
git add Docs\superpowers\verification\2026-05-06-frontend-login-ui-run.md
git commit -m "docs: verify frontend login ui flow"
```

---

## Self-Review

Spec coverage:

- Startup/login/guest/character list/create/main lobby are covered by Tasks 1-5.
- Dedicated Server and client debugging are covered by Task 6.
- `E:\UnrealEngine-5.7.1-release` is used in every build and run command.
- Existing duplicate UI assets are not moved or deleted.
- Compile blockers are addressed before UI and runtime validation.

Placeholder scan:

- No unresolved placeholder markers remain.
- Binary Blueprint work is explicit and limited to named assets.

Type consistency:

- `EDBALoginFlowState` is defined by `UDBALoginFlowSubsystem`.
- `EDBAFrontendPage` is introduced by `UDBAFrontendRootWidgetBase` and used only for page presentation.
- Existing controller names match current source files.
