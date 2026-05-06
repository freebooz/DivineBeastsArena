# 登录闭环 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 打通 `启动 -> 登录 -> 角色列表 -> 创建角色 -> 大厅` 的第一版可玩闭环，真实后端优先，Mock 兜底。

**Architecture:** 账号能力统一挂在 `UDBAAccountServiceBase` 下。新增 `UDBAOnlineAccountService` 负责 HTTP 后端协议和兜底调度，复用 `UDBAMockAccountService` 保障开发闭环；新增前台登录流程子系统协调自动登录、角色列表、角色创建和大厅状态。

**Tech Stack:** Unreal Engine 5.7 C++、UGameInstanceSubsystem、HTTP Module、Json/JsonUtilities、UMG Blueprint 绑定、Unreal Automation Tests。

---

## Scope Check

本计划只实现登录闭环。它不实现真实后端服务本身，不实现完整美术 UI，不实现支付、好友、防沉迷、排行榜、跨平台账号绑定，也不把对局运行时状态写入账号存档。

当前工作区已有 `Source/GameCore` 和 `Source/GameMoba` 模块拆分改动。登录实现依赖 `GameCore/Account` 可编译，因此第一项任务先完成账号相关模块归属和构建依赖清理。

## File Structure

- Modify: `DivineBeastsArena.uproject`  
  保留 `GameCore` 模块声明，确保运行时加载。
- Modify: `Source/GameCore/GameCore.Build.cs`  
  增加 `UMG` 公共依赖，增加 `HTTP`、`Json`、`JsonUtilities` 私有依赖。
- Modify: `Source/GameCore/Public/GameCore/**/*.h`  
  将从 `DivineBeastsArena` 拆到 `GameCore` 的类型导出宏改为 `GAMECORE_API`。
- Create: `Source/GameCore/Private/GameCore/Core/DBALogChannels.cpp`  
  让 `GameCore` 自己定义 `LogDBACore` 等日志分类。
- Delete after migration verification: `Source/DivineBeastsArena/Public/GameCore/**` and `Source/DivineBeastsArena/Private/GameCore/**`  
  避免 UHT 看到同名反射类型的重复定义。
- Modify: `Source/GameCore/Public/GameCore/Account/DBAAccountTypes.h`  
  为角色摘要增加 `PrimaryElement`、`FiveCamp`、`FixedSkillGroupId` 等闭环字段，保持旧字段兼容。
- Modify: `Source/GameCore/Public/GameCore/Account/DBAAccountSaveGame.h` and `Source/GameCore/Private/GameCore/Account/DBAAccountSaveGame.cpp`  
  让存档校验覆盖新增角色字段。
- Create: `Source/GameCore/Public/GameCore/Account/DBAOnlineAccountTypes.h`  
  定义在线账号服务配置、请求来源、兜底原因、在线错误分类。
- Create: `Source/GameCore/Private/GameCore/Account/DBAOnlineAccountJson.h`
- Create: `Source/GameCore/Private/GameCore/Account/DBAOnlineAccountJson.cpp`  
  负责 JSON 和项目结构体之间的转换。
- Create: `Source/GameCore/Public/GameCore/Account/DBAOnlineAccountService.h`
- Create: `Source/GameCore/Private/GameCore/Account/DBAOnlineAccountService.cpp`  
  实现在线登录、注册、Token 刷新、角色列表、角色创建、角色选择、Mock 兜底。
- Create: `Source/GameCore/Public/GameCore/Session/DBALoginFlowSubsystem.h`
- Create: `Source/GameCore/Private/GameCore/Session/DBALoginFlowSubsystem.cpp`  
  协调启动、自动登录、登录、角色列表、角色选择、角色创建、大厅状态。
- Modify: `Source/GameCore/Public/GameCore/Session/DBAFrontendSessionSubsystem.h`
- Modify: `Source/GameCore/Private/GameCore/Session/DBAFrontendSessionSubsystem.cpp`  
  为登录流程提供显式 `LoggingIn`、`MainLobby` 状态切换入口。
- Create: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBALoginWidgetController.h`
- Create: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBALoginWidgetController.cpp`
- Create: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBACharacterSelectWidgetController.h`
- Create: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterSelectWidgetController.cpp`
- Create: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.h`
- Create: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.cpp`
  提供 Blueprint 可调用接口和事件，现有 UMG 资产绑定这些控制器。
- Create: `Source/GameCore/Private/Tests/DBAOnlineAccountJsonTests.cpp`
- Create: `Source/GameCore/Private/Tests/DBAOnlineAccountServiceTests.cpp`
- Create: `Source/GameCore/Private/Tests/DBALoginFlowSubsystemTests.cpp`
  覆盖 JSON、兜底策略、无角色进入创建、有角色进入选择、创建后进入大厅。

---

### Task 1: 稳定 GameCore 账号模块归属

**Files:**
- Modify: `Source/GameCore/GameCore.Build.cs`
- Modify: `Source/GameCore/Public/GameCore/**/*.h`
- Create: `Source/GameCore/Private/GameCore/Core/DBALogChannels.cpp`
- Delete: `Source/DivineBeastsArena/Public/GameCore/**`
- Delete: `Source/DivineBeastsArena/Private/GameCore/**`

- [ ] **Step 1: 修改 GameCore 构建依赖**

将 `Source/GameCore/GameCore.Build.cs` 调整为：

```csharp
// Copyright Freebooz Games, Inc. All Rights Reserved.
using UnrealBuildTool;

public class GameCore : ModuleRules
{
	public GameCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"HTTP",
			"Json",
			"JsonUtilities"
		});

		PublicIncludePaths.AddRange(new string[]
		{
			"GameCore/Public"
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			"GameCore/Private"
		});

		bUseUnity = true;
		bLegacyPublicIncludePaths = false;
		CppStandard = CppStandardVersion.Cpp20;
	}
}
```

- [ ] **Step 2: 替换 GameCore 导出宏**

运行：

```powershell
Get-ChildItem Source\GameCore -Recurse -Include *.h,*.cpp |
  ForEach-Object {
    (Get-Content $_.FullName) -replace 'DIVINEBEASTSARENA_API', 'GAMECORE_API' |
      Set-Content $_.FullName
  }
```

预期：`Source/GameCore` 下不再出现 `DIVINEBEASTSARENA_API`。

验证：

```powershell
rg -n "DIVINEBEASTSARENA_API" Source\GameCore
```

预期：无输出。

- [ ] **Step 3: 移动日志定义到 GameCore**

创建 `Source/GameCore/Private/GameCore/Core/DBALogChannels.cpp`：

```cpp
// Copyright FreeboozStudio. All Rights Reserved.

#include "GameCore/Core/DBALogChannels.h"

DEFINE_LOG_CATEGORY(LogDBACore);
DEFINE_LOG_CATEGORY(LogDBAFrontend);
DEFINE_LOG_CATEGORY(LogDBAMatch);
DEFINE_LOG_CATEGORY(LogDBACombat);
DEFINE_LOG_CATEGORY(LogDBAUI);
DEFINE_LOG_CATEGORY(LogDBAData);
DEFINE_LOG_CATEGORY(LogDBANetwork);
DEFINE_LOG_CATEGORY(LogDBAValidation);
DEFINE_LOG_CATEGORY(LogDBAAI);
DEFINE_LOG_CATEGORY(LogDBATelemetry);
DEFINE_LOG_CATEGORY(LogDBAGameOps);
```

- [ ] **Step 4: 删除旧 GameCore 编译副本**

确认 `Source/GameCore/Public/GameCore/Account/DBAAccountTypes.h` 存在：

```powershell
Test-Path Source\GameCore\Public\GameCore\Account\DBAAccountTypes.h
```

预期：`True`

删除旧副本：

```powershell
Remove-Item -Recurse -Force Source\DivineBeastsArena\Public\GameCore
Remove-Item -Recurse -Force Source\DivineBeastsArena\Private\GameCore
```

验证：

```powershell
rg --files Source\DivineBeastsArena\Public\GameCore Source\DivineBeastsArena\Private\GameCore
```

预期：路径不存在或无输出。

- [ ] **Step 5: 验证构建**

运行：

```powershell
& "$env:UE5_ROOT\Engine\Build\BatchFiles\Build.bat" DivineBeastsArenaEditor Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex
```

预期：构建成功，没有重复 `UCLASS`、重复 `UENUM`、无法解析 `LogDBACore`、无法解析 `GAMECORE_API` 的错误。

- [ ] **Step 6: 提交**

```powershell
git add Source\GameCore Source\DivineBeastsArena\Public\GameCore Source\DivineBeastsArena\Private\GameCore
git commit -m "refactor: make GameCore own account systems"
```

---

### Task 2: 扩展角色摘要以承接 v4/v4.1 创建数据

**Files:**
- Modify: `Source/GameCore/Public/GameCore/Account/DBAAccountTypes.h`
- Modify: `Source/GameCore/Private/GameCore/Account/DBAAccountSaveGame.cpp`
- Test: `Source/GameCore/Private/Tests/DBAAccountSaveGameTests.cpp`

- [ ] **Step 1: 写存档字段测试**

创建 `Source/GameCore/Private/Tests/DBAAccountSaveGameTests.cpp`：

```cpp
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GameCore/Account/DBAAccountSaveGame.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAAccountSaveGameCharacterBuildFieldsTest,
	"DivineBeastsArena.GameCore.Account.SaveGame.CharacterBuildFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAAccountSaveGameCharacterBuildFieldsTest::RunTest(const FString& Parameters)
{
	UDBAAccountSaveGame* SaveGame = NewObject<UDBAAccountSaveGame>();
	TestNotNull(TEXT("SaveGame should be created"), SaveGame);

	FDBACharacterSummary Summary;
	Summary.CharacterId = FDBACharacterId(TEXT("char_001"));
	Summary.CharacterName = TEXT("水灵鼠");
	Summary.DefaultZodiac = EDBAZodiac::Rat;
	Summary.DefaultElement = EDBAElement::Water;
	Summary.DefaultFiveCamp = EDBAFiveCamp::West;
	Summary.PrimaryElement = EDBAElement::Water;
	Summary.FiveCamp = EDBAFiveCamp::West;
	Summary.FixedSkillGroupId = TEXT("FSG_Rat_Water");
	Summary.BaseAttributeTemplateId = TEXT("BAT_Rat_Water");

	TestTrue(TEXT("Character should be valid"), Summary.IsValid());
	TestTrue(TEXT("Character should be added"), SaveGame->AddCharacter(Summary));

	const FDBACharacterSummary* Found = SaveGame->FindCharacter(FDBACharacterId(TEXT("char_001")));
	TestNotNull(TEXT("Saved character should be found"), Found);
	if (Found)
	{
		TestEqual(TEXT("PrimaryElement should persist"), Found->PrimaryElement, EDBAElement::Water);
		TestEqual(TEXT("FiveCamp should persist"), Found->FiveCamp, EDBAFiveCamp::West);
		TestEqual(TEXT("FixedSkillGroupId should persist"), Found->FixedSkillGroupId, FString(TEXT("FSG_Rat_Water")));
		TestEqual(TEXT("BaseAttributeTemplateId should persist"), Found->BaseAttributeTemplateId, FString(TEXT("BAT_Rat_Water")));
	}

	return true;
}

#endif
```

- [ ] **Step 2: 运行测试确认失败**

```powershell
& "$env:UE5_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.GameCore.Account.SaveGame.CharacterBuildFields; Quit" -unattended -nop4 -nosplash
```

预期：编译失败或测试失败，提示 `PrimaryElement`、`FiveCamp`、`FixedSkillGroupId`、`BaseAttributeTemplateId` 不存在。

- [ ] **Step 3: 增加角色摘要字段**

在 `FDBACharacterSummary` 中 `DefaultFiveCamp` 后加入：

```cpp
/** 创建时选择的主元素；用于固定技能组和属性模板生成 */
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
EDBAElement PrimaryElement = EDBAElement::None;

/** 创建时选择的五方阵营；仅影响外观和特效风格 */
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
EDBAFiveCamp FiveCamp = EDBAFiveCamp::None;

/** 固定技能组 ID，由 Zodiac + PrimaryElement 生成 */
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
FString FixedSkillGroupId;

/** 基础属性模板 ID，用于进入战斗时初始化 8 个核心属性 */
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
FString BaseAttributeTemplateId;
```

- [ ] **Step 4: 更新 Mock 创建角色默认写入**

在 `UDBAMockAccountService::CreateCharacter` 创建 `Summary` 后补充：

```cpp
Summary.PrimaryElement = Request.DefaultElement;
Summary.FiveCamp = Request.DefaultFiveCamp;
Summary.FixedSkillGroupId = FString::Printf(
	TEXT("FSG_%s_%s"),
	*UEnum::GetValueAsString(Request.DefaultZodiac).RightChop(FString(TEXT("EDBAZodiac::")).Len()),
	*UEnum::GetValueAsString(Request.DefaultElement).RightChop(FString(TEXT("EDBAElement::")).Len()));
Summary.BaseAttributeTemplateId = FString::Printf(
	TEXT("BAT_%s_%s"),
	*UEnum::GetValueAsString(Request.DefaultZodiac).RightChop(FString(TEXT("EDBAZodiac::")).Len()),
	*UEnum::GetValueAsString(Request.DefaultElement).RightChop(FString(TEXT("EDBAElement::")).Len()));
```

- [ ] **Step 5: 运行测试确认通过**

```powershell
& "$env:UE5_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.GameCore.Account.SaveGame.CharacterBuildFields; Quit" -unattended -nop4 -nosplash
```

预期：测试通过。

- [ ] **Step 6: 提交**

```powershell
git add Source\GameCore\Public\GameCore\Account\DBAAccountTypes.h Source\GameCore\Private\GameCore\Account\DBAMockAccountService.cpp Source\GameCore\Private\Tests\DBAAccountSaveGameTests.cpp
git commit -m "feat: persist character build selections"
```

---

### Task 3: 添加在线账号 JSON 协议转换

**Files:**
- Create: `Source/GameCore/Public/GameCore/Account/DBAOnlineAccountTypes.h`
- Create: `Source/GameCore/Private/GameCore/Account/DBAOnlineAccountJson.h`
- Create: `Source/GameCore/Private/GameCore/Account/DBAOnlineAccountJson.cpp`
- Test: `Source/GameCore/Private/Tests/DBAOnlineAccountJsonTests.cpp`

- [ ] **Step 1: 写 JSON 解析测试**

创建 `Source/GameCore/Private/Tests/DBAOnlineAccountJsonTests.cpp`：

```cpp
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "GameCore/Account/DBAOnlineAccountJson.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonLoginTest,
	"DivineBeastsArena.GameCore.Account.OnlineJson.ParseLoginResponse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAOnlineAccountJsonLoginTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"success": true,
		"token": "session-token",
		"account": {
			"accountId": "account_001",
			"displayName": "玩家001",
			"loginType": "Email",
			"status": "Normal",
			"level": 7,
			"experience": 120
		}
	})");

	FDBALoginResponse Response;
	FString Error;
	TestTrue(TEXT("Login response should parse"), FDBAOnlineAccountJson::ParseLoginResponse(Json, Response, Error));
	TestTrue(TEXT("Login should succeed"), Response.bSuccess);
	TestEqual(TEXT("Token should parse"), Response.SessionToken, FString(TEXT("session-token")));
	TestEqual(TEXT("AccountId should parse"), Response.AccountInfo.AccountId.ToString(), FString(TEXT("account_001")));
	TestEqual(TEXT("DisplayName should parse"), Response.AccountInfo.DisplayName, FString(TEXT("玩家001")));
	TestEqual(TEXT("LoginType should parse"), Response.AccountInfo.LoginType, EDBALoginType::Email);
	TestEqual(TEXT("Status should parse"), Response.AccountInfo.Status, EDBAAccountStatus::Normal);
	TestEqual(TEXT("Level should parse"), Response.AccountInfo.Level, 7);
	TestEqual(TEXT("Experience should parse"), Response.AccountInfo.Experience, 120);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonCharactersTest,
	"DivineBeastsArena.GameCore.Account.OnlineJson.ParseCharacters",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAOnlineAccountJsonCharactersTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"success": true,
		"characters": [
			{
				"characterId": "char_001",
				"characterName": "水灵鼠",
				"zodiac": "Rat",
				"primaryElement": "Water",
				"fiveCamp": "West",
				"fixedSkillGroupId": "FSG_Rat_Water",
				"baseAttributeTemplateId": "BAT_Rat_Water",
				"level": 3
			}
		]
	})");

	TArray<FDBACharacterSummary> Characters;
	FString Error;
	TestTrue(TEXT("Characters should parse"), FDBAOnlineAccountJson::ParseCharacterListResponse(Json, Characters, Error));
	TestEqual(TEXT("Character count"), Characters.Num(), 1);
	if (Characters.Num() == 1)
	{
		TestEqual(TEXT("CharacterId"), Characters[0].CharacterId.ToString(), FString(TEXT("char_001")));
		TestEqual(TEXT("CharacterName"), Characters[0].CharacterName, FString(TEXT("水灵鼠")));
		TestEqual(TEXT("Zodiac"), Characters[0].DefaultZodiac, EDBAZodiac::Rat);
		TestEqual(TEXT("PrimaryElement"), Characters[0].PrimaryElement, EDBAElement::Water);
		TestEqual(TEXT("FiveCamp"), Characters[0].FiveCamp, EDBAFiveCamp::West);
		TestEqual(TEXT("FixedSkillGroupId"), Characters[0].FixedSkillGroupId, FString(TEXT("FSG_Rat_Water")));
	}
	return true;
}

#endif
```

- [ ] **Step 2: 运行测试确认失败**

```powershell
& "$env:UE5_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.GameCore.Account.OnlineJson; Quit" -unattended -nop4 -nosplash
```

预期：编译失败，提示 `DBAOnlineAccountJson.h` 不存在。

- [ ] **Step 3: 创建在线类型头文件**

创建 `Source/GameCore/Public/GameCore/Account/DBAOnlineAccountTypes.h`：

```cpp
// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DBAOnlineAccountTypes.generated.h"

UENUM(BlueprintType)
enum class EDBAOnlineAccountError : uint8
{
	None,
	NetworkUnavailable,
	Timeout,
	EndpointMissing,
	ServiceUnavailable,
	InvalidCredentials,
	AccountUnavailable,
	TokenExpired,
	ValidationFailed,
	MalformedResponse
};

USTRUCT(BlueprintType)
struct GAMECORE_API FDBAOnlineAccountConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account|Online")
	FString ServerHost = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account|Online")
	int32 ServerPort = 8080;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account|Online")
	float RequestTimeoutSeconds = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account|Online")
	bool bAllowMockFallback = true;

	FString GetBaseUrl() const
	{
		return FString::Printf(TEXT("http://%s:%d"), *ServerHost, ServerPort);
	}
};
```

- [ ] **Step 4: 创建 JSON 转换接口**

创建 `Source/GameCore/Private/GameCore/Account/DBAOnlineAccountJson.h`：

```cpp
// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountTypes.h"

class FDBAOnlineAccountJson
{
public:
	static FString BuildLoginRequest(const FDBALoginRequest& Request);
	static FString BuildCreateCharacterRequest(const FDBACharacterCreateRequest& Request);
	static bool ParseLoginResponse(const FString& Json, FDBALoginResponse& OutResponse, FString& OutError);
	static bool ParseCharacterListResponse(const FString& Json, TArray<FDBACharacterSummary>& OutCharacters, FString& OutError);
	static bool ParseCreateCharacterResponse(const FString& Json, FDBACharacterCreateResponse& OutResponse, FString& OutError);

private:
	static EDBALoginType ParseLoginType(const FString& Value);
	static EDBAAccountStatus ParseAccountStatus(const FString& Value);
	static EDBAZodiac ParseZodiac(const FString& Value);
	static EDBAElement ParseElement(const FString& Value);
	static EDBAFiveCamp ParseFiveCamp(const FString& Value);
	static FString ToString(EDBAZodiac Value);
	static FString ToString(EDBAElement Value);
	static FString ToString(EDBAFiveCamp Value);
};
```

- [ ] **Step 5: 实现 JSON 转换**

创建 `Source/GameCore/Private/GameCore/Account/DBAOnlineAccountJson.cpp`，实现对象读写：

```cpp
// Copyright FreeboozStudio. All Rights Reserved.

#include "GameCore/Account/DBAOnlineAccountJson.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	bool ParseObject(const FString& Json, TSharedPtr<FJsonObject>& OutObject, FString& OutError)
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		if (!FJsonSerializer::Deserialize(Reader, OutObject) || !OutObject.IsValid())
		{
			OutError = TEXT("响应 JSON 格式无效");
			return false;
		}
		return true;
	}

	FString WriteObject(const TSharedRef<FJsonObject>& Object)
	{
		FString Output;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
		FJsonSerializer::Serialize(Object, Writer);
		return Output;
	}
}

FString FDBAOnlineAccountJson::BuildLoginRequest(const FDBALoginRequest& Request)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("email"), Request.Email);
	Object->SetStringField(TEXT("password"), Request.Password);
	Object->SetStringField(TEXT("deviceId"), Request.DeviceId);
	return WriteObject(Object);
}

FString FDBAOnlineAccountJson::BuildCreateCharacterRequest(const FDBACharacterCreateRequest& Request)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("characterName"), Request.CharacterName);
	Object->SetStringField(TEXT("zodiac"), ToString(Request.DefaultZodiac));
	Object->SetStringField(TEXT("primaryElement"), ToString(Request.DefaultElement));
	Object->SetStringField(TEXT("fiveCamp"), ToString(Request.DefaultFiveCamp));
	return WriteObject(Object);
}

bool FDBAOnlineAccountJson::ParseLoginResponse(const FString& Json, FDBALoginResponse& OutResponse, FString& OutError)
{
	TSharedPtr<FJsonObject> Object;
	if (!ParseObject(Json, Object, OutError))
	{
		return false;
	}

	OutResponse.bSuccess = Object->GetBoolField(TEXT("success"));
	OutResponse.ErrorMessage = Object->GetStringField(TEXT("errorMessage"));
	if (!OutResponse.bSuccess)
	{
		return true;
	}

	OutResponse.SessionToken = Object->GetStringField(TEXT("token"));
	const TSharedPtr<FJsonObject>* AccountObject = nullptr;
	if (!Object->TryGetObjectField(TEXT("account"), AccountObject) || !AccountObject || !AccountObject->IsValid())
	{
		OutError = TEXT("登录响应缺少 account");
		return false;
	}

	OutResponse.AccountInfo.AccountId = FDBAAccountId((*AccountObject)->GetStringField(TEXT("accountId")));
	OutResponse.AccountInfo.DisplayName = (*AccountObject)->GetStringField(TEXT("displayName"));
	OutResponse.AccountInfo.LoginType = ParseLoginType((*AccountObject)->GetStringField(TEXT("loginType")));
	OutResponse.AccountInfo.Status = ParseAccountStatus((*AccountObject)->GetStringField(TEXT("status")));
	OutResponse.AccountInfo.Level = (*AccountObject)->GetIntegerField(TEXT("level"));
	OutResponse.AccountInfo.Experience = (*AccountObject)->GetIntegerField(TEXT("experience"));
	OutResponse.AccountInfo.LastLoginTime = FDateTime::UtcNow().ToUnixTimestamp();
	return true;
}

bool FDBAOnlineAccountJson::ParseCharacterListResponse(const FString& Json, TArray<FDBACharacterSummary>& OutCharacters, FString& OutError)
{
	TSharedPtr<FJsonObject> Object;
	if (!ParseObject(Json, Object, OutError))
	{
		return false;
	}

	if (!Object->GetBoolField(TEXT("success")))
	{
		OutError = Object->GetStringField(TEXT("errorMessage"));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
	if (!Object->TryGetArrayField(TEXT("characters"), Items) || !Items)
	{
		OutError = TEXT("角色列表响应缺少 characters");
		return false;
	}

	OutCharacters.Reset();
	for (const TSharedPtr<FJsonValue>& Item : *Items)
	{
		const TSharedPtr<FJsonObject> CharacterObject = Item->AsObject();
		if (!CharacterObject.IsValid())
		{
			continue;
		}

		FDBACharacterSummary Summary;
		Summary.CharacterId = FDBACharacterId(CharacterObject->GetStringField(TEXT("characterId")));
		Summary.CharacterName = CharacterObject->GetStringField(TEXT("characterName"));
		Summary.DefaultZodiac = ParseZodiac(CharacterObject->GetStringField(TEXT("zodiac")));
		Summary.DefaultElement = ParseElement(CharacterObject->GetStringField(TEXT("primaryElement")));
		Summary.DefaultFiveCamp = ParseFiveCamp(CharacterObject->GetStringField(TEXT("fiveCamp")));
		Summary.PrimaryElement = Summary.DefaultElement;
		Summary.FiveCamp = Summary.DefaultFiveCamp;
		Summary.FixedSkillGroupId = CharacterObject->GetStringField(TEXT("fixedSkillGroupId"));
		Summary.BaseAttributeTemplateId = CharacterObject->GetStringField(TEXT("baseAttributeTemplateId"));
		Summary.Level = CharacterObject->GetIntegerField(TEXT("level"));
		OutCharacters.Add(Summary);
	}

	return true;
}

bool FDBAOnlineAccountJson::ParseCreateCharacterResponse(const FString& Json, FDBACharacterCreateResponse& OutResponse, FString& OutError)
{
	TArray<FDBACharacterSummary> Characters;
	if (!ParseCharacterListResponse(Json.Replace(TEXT("\"character\":"), TEXT("\"characters\":[")) + TEXT("]"), Characters, OutError))
	{
		return false;
	}

	OutResponse.bSuccess = Characters.Num() > 0;
	if (OutResponse.bSuccess)
	{
		OutResponse.CharacterSummary = Characters[0];
	}
	return true;
}

EDBALoginType FDBAOnlineAccountJson::ParseLoginType(const FString& Value)
{
	return Value.Equals(TEXT("Email"), ESearchCase::IgnoreCase) ? EDBALoginType::Email : EDBALoginType::Guest;
}

EDBAAccountStatus FDBAOnlineAccountJson::ParseAccountStatus(const FString& Value)
{
	if (Value.Equals(TEXT("Banned"), ESearchCase::IgnoreCase)) return EDBAAccountStatus::Banned;
	if (Value.Equals(TEXT("Frozen"), ESearchCase::IgnoreCase)) return EDBAAccountStatus::Frozen;
	if (Value.Equals(TEXT("PendingVerification"), ESearchCase::IgnoreCase)) return EDBAAccountStatus::PendingVerification;
	return EDBAAccountStatus::Normal;
}

EDBAZodiac FDBAOnlineAccountJson::ParseZodiac(const FString& Value)
{
	if (Value.Equals(TEXT("Rat"), ESearchCase::IgnoreCase)) return EDBAZodiac::Rat;
	if (Value.Equals(TEXT("Ox"), ESearchCase::IgnoreCase)) return EDBAZodiac::Ox;
	if (Value.Equals(TEXT("Tiger"), ESearchCase::IgnoreCase)) return EDBAZodiac::Tiger;
	if (Value.Equals(TEXT("Rabbit"), ESearchCase::IgnoreCase)) return EDBAZodiac::Rabbit;
	if (Value.Equals(TEXT("Dragon"), ESearchCase::IgnoreCase)) return EDBAZodiac::Dragon;
	if (Value.Equals(TEXT("Snake"), ESearchCase::IgnoreCase)) return EDBAZodiac::Snake;
	if (Value.Equals(TEXT("Horse"), ESearchCase::IgnoreCase)) return EDBAZodiac::Horse;
	if (Value.Equals(TEXT("Goat"), ESearchCase::IgnoreCase)) return EDBAZodiac::Goat;
	if (Value.Equals(TEXT("Monkey"), ESearchCase::IgnoreCase)) return EDBAZodiac::Monkey;
	if (Value.Equals(TEXT("Rooster"), ESearchCase::IgnoreCase)) return EDBAZodiac::Rooster;
	if (Value.Equals(TEXT("Dog"), ESearchCase::IgnoreCase)) return EDBAZodiac::Dog;
	if (Value.Equals(TEXT("Pig"), ESearchCase::IgnoreCase)) return EDBAZodiac::Pig;
	return EDBAZodiac::None;
}

EDBAElement FDBAOnlineAccountJson::ParseElement(const FString& Value)
{
	if (Value.Equals(TEXT("Gold"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("Metal"), ESearchCase::IgnoreCase)) return EDBAElement::Gold;
	if (Value.Equals(TEXT("Wood"), ESearchCase::IgnoreCase)) return EDBAElement::Wood;
	if (Value.Equals(TEXT("Water"), ESearchCase::IgnoreCase)) return EDBAElement::Water;
	if (Value.Equals(TEXT("Fire"), ESearchCase::IgnoreCase)) return EDBAElement::Fire;
	if (Value.Equals(TEXT("Earth"), ESearchCase::IgnoreCase)) return EDBAElement::Earth;
	return EDBAElement::None;
}

EDBAFiveCamp FDBAOnlineAccountJson::ParseFiveCamp(const FString& Value)
{
	if (Value.Equals(TEXT("East"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("AzureDragon"), ESearchCase::IgnoreCase)) return EDBAFiveCamp::East;
	if (Value.Equals(TEXT("West"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("WhiteTiger"), ESearchCase::IgnoreCase)) return EDBAFiveCamp::West;
	if (Value.Equals(TEXT("South"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("VermilionBird"), ESearchCase::IgnoreCase)) return EDBAFiveCamp::South;
	if (Value.Equals(TEXT("North"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("BlackTortoise"), ESearchCase::IgnoreCase)) return EDBAFiveCamp::North;
	if (Value.Equals(TEXT("Center"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("Kirin"), ESearchCase::IgnoreCase)) return EDBAFiveCamp::Center;
	return EDBAFiveCamp::None;
}

FString FDBAOnlineAccountJson::ToString(EDBAZodiac Value)
{
	return UEnum::GetValueAsString(Value).RightChop(FString(TEXT("EDBAZodiac::")).Len());
}

FString FDBAOnlineAccountJson::ToString(EDBAElement Value)
{
	return UEnum::GetValueAsString(Value).RightChop(FString(TEXT("EDBAElement::")).Len());
}

FString FDBAOnlineAccountJson::ToString(EDBAFiveCamp Value)
{
	return UEnum::GetValueAsString(Value).RightChop(FString(TEXT("EDBAFiveCamp::")).Len());
}
```

- [ ] **Step 6: 运行测试确认通过**

```powershell
& "$env:UE5_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.GameCore.Account.OnlineJson; Quit" -unattended -nop4 -nosplash
```

预期：两个 JSON 测试通过。

- [ ] **Step 7: 提交**

```powershell
git add Source\GameCore\Public\GameCore\Account\DBAOnlineAccountTypes.h Source\GameCore\Private\GameCore\Account\DBAOnlineAccountJson.h Source\GameCore\Private\GameCore\Account\DBAOnlineAccountJson.cpp Source\GameCore\Private\Tests\DBAOnlineAccountJsonTests.cpp
git commit -m "feat: add online account json protocol"
```

---

### Task 4: 实现 UDBAOnlineAccountService 和 Mock 兜底

**Files:**
- Create: `Source/GameCore/Public/GameCore/Account/DBAOnlineAccountService.h`
- Create: `Source/GameCore/Private/GameCore/Account/DBAOnlineAccountService.cpp`
- Test: `Source/GameCore/Private/Tests/DBAOnlineAccountServiceTests.cpp`

- [ ] **Step 1: 写兜底策略测试**

创建 `Source/GameCore/Private/Tests/DBAOnlineAccountServiceTests.cpp`：

```cpp
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GameCore/Account/DBAOnlineAccountTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineFallbackPolicyTest,
	"DivineBeastsArena.GameCore.Account.OnlineService.FallbackPolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAOnlineFallbackPolicyTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("NetworkUnavailable can fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::NetworkUnavailable));
	TestTrue(TEXT("Timeout can fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::Timeout));
	TestTrue(TEXT("EndpointMissing can fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::EndpointMissing));
	TestTrue(TEXT("ServiceUnavailable can fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::ServiceUnavailable));
	TestFalse(TEXT("InvalidCredentials cannot fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::InvalidCredentials));
	TestFalse(TEXT("AccountUnavailable cannot fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::AccountUnavailable));
	TestFalse(TEXT("ValidationFailed cannot fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::ValidationFailed));
	return true;
}

#endif
```

- [ ] **Step 2: 运行测试确认失败**

```powershell
& "$env:UE5_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.GameCore.Account.OnlineService.FallbackPolicy; Quit" -unattended -nop4 -nosplash
```

预期：编译失败，提示 `UDBAOnlineAccountService` 不存在。

- [ ] **Step 3: 创建服务头文件**

创建 `Source/GameCore/Public/GameCore/Account/DBAOnlineAccountService.h`：

```cpp
// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountServiceBase.h"
#include "GameCore/Account/DBAOnlineAccountTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "DBAOnlineAccountService.generated.h"

class UDBAMockAccountService;

UCLASS()
class GAMECORE_API UDBAOnlineAccountService : public UDBAAccountServiceBase
{
	GENERATED_BODY()

public:
	UDBAOnlineAccountService();

	virtual void OnSubsystemInitialize() override;
	virtual void Login(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete) override;
	virtual void Register(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete) override;
	virtual void GuestLogin(FDBAOnLoginComplete OnComplete) override;
	virtual void AutoLogin(FDBAOnLoginComplete OnComplete) override;
	virtual void GetCharacterList(FDBAOnCharacterListLoaded OnComplete) override;
	virtual void CreateCharacter(const FDBACharacterCreateRequest& Request, FDBAOnCharacterCreated OnComplete) override;
	virtual void SelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete) override;

	static bool CanFallbackToMock(EDBAOnlineAccountError Error);

protected:
	UPROPERTY()
	FDBAOnlineAccountConfig OnlineConfig;

	UPROPERTY()
	TObjectPtr<UDBAMockAccountService> MockService;

	FString BuildUrl(const FString& Path) const;
	TSharedRef<IHttpRequest> CreateJsonRequest(const FString& Verb, const FString& Path, const FString& Body);
	void CacheLoginSuccess(const FDBALoginResponse& Response);
	void FallbackLogin(FDBAOnLoginComplete OnComplete);
	void FallbackAutoLogin(FDBAOnLoginComplete OnComplete);
	void FallbackCharacterList(FDBAOnCharacterListLoaded OnComplete);
	void FallbackCreateCharacter(const FDBACharacterCreateRequest& Request, FDBAOnCharacterCreated OnComplete);
};
```

- [ ] **Step 4: 实现兜底策略和配置读取**

创建 `Source/GameCore/Private/GameCore/Account/DBAOnlineAccountService.cpp`，先实现初始化和兜底判断：

```cpp
// Copyright FreeboozStudio. All Rights Reserved.

#include "GameCore/Account/DBAOnlineAccountService.h"
#include "GameCore/Account/DBAMockAccountService.h"
#include "GameCore/Account/DBAAccountSaveGame.h"
#include "GameCore/Account/DBAOnlineAccountJson.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

UDBAOnlineAccountService::UDBAOnlineAccountService()
{
}

void UDBAOnlineAccountService::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

	GConfig->GetString(TEXT("/Script/DivineBeastsArena.DBAAccountSettings"), TEXT("DefaultLoginServer"), OnlineConfig.ServerHost, GGameIni);
	GConfig->GetInt(TEXT("/Script/DivineBeastsArena.DBAAccountSettings"), TEXT("DefaultLoginPort"), OnlineConfig.ServerPort, GGameIni);

	if (UGameInstance* GI = GetGameInstance())
	{
		MockService = GI->GetSubsystem<UDBAMockAccountService>();
	}
}

bool UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError Error)
{
	return Error == EDBAOnlineAccountError::NetworkUnavailable
		|| Error == EDBAOnlineAccountError::Timeout
		|| Error == EDBAOnlineAccountError::EndpointMissing
		|| Error == EDBAOnlineAccountError::ServiceUnavailable;
}

FString UDBAOnlineAccountService::BuildUrl(const FString& Path) const
{
	return OnlineConfig.GetBaseUrl() + Path;
}

TSharedRef<IHttpRequest> UDBAOnlineAccountService::CreateJsonRequest(const FString& Verb, const FString& Path, const FString& Body)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(Verb);
	Request->SetURL(BuildUrl(Path));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetTimeout(OnlineConfig.RequestTimeoutSeconds);
	if (!SessionToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *SessionToken));
	}
	Request->SetContentAsString(Body);
	return Request;
}

void UDBAOnlineAccountService::CacheLoginSuccess(const FDBALoginResponse& Response)
{
	CurrentAccountInfo = Response.AccountInfo;
	SessionToken = Response.SessionToken;

	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame)
	{
		SaveGame = CreateDefaultAccountSaveGame();
	}
	if (SaveGame)
	{
		SaveGame->AccountInfo = CurrentAccountInfo;
		SaveGame->SessionToken = SessionToken;
		SaveAccountSaveGame(SaveGame);
	}
}
```

- [ ] **Step 5: 实现登录和兜底登录**

在同一 `.cpp` 中加入：

```cpp
void UDBAOnlineAccountService::Login(const FDBALoginRequest& RequestData, FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Login")))
	{
		return;
	}

	const FString Body = FDBAOnlineAccountJson::BuildLoginRequest(RequestData);
	TSharedRef<IHttpRequest> Request = CreateJsonRequest(TEXT("POST"), TEXT("/api/auth/login"), Body);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, OnComplete](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		if (!bSucceeded || !HttpResponse.IsValid())
		{
			LogSubsystemWarning(TEXT("[Account] Online unavailable, fallback to mock"));
			FallbackLogin(OnComplete);
			return;
		}

		const int32 Code = HttpResponse->GetResponseCode();
		if (Code == 404 || Code == 501 || Code == 503)
		{
			LogSubsystemWarning(TEXT("[Account] Online unavailable, fallback to mock"));
			FallbackLogin(OnComplete);
			return;
		}

		FDBALoginResponse Response;
		FString Error;
		if (!FDBAOnlineAccountJson::ParseLoginResponse(HttpResponse->GetContentAsString(), Response, Error))
		{
			Response.bSuccess = false;
			Response.ErrorMessage = Error;
			OnComplete.ExecuteIfBound(Response);
			return;
		}

		if (Response.bSuccess)
		{
			LogSubsystemInfo(TEXT("[Account] Online login succeeded"));
			CacheLoginSuccess(Response);
		}
		else
		{
			LogSubsystemWarning(FString::Printf(TEXT("[Account] Login failed: %s"), *Response.ErrorMessage));
		}

		OnComplete.ExecuteIfBound(Response);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::FallbackLogin(FDBAOnLoginComplete OnComplete)
{
	if (OnlineConfig.bAllowMockFallback && MockService)
	{
		MockService->GuestLogin(OnComplete);
		return;
	}

	FDBALoginResponse Response;
	Response.bSuccess = false;
	Response.ErrorMessage = TEXT("服务器不可用");
	OnComplete.ExecuteIfBound(Response);
}
```

- [ ] **Step 6: 实现注册、自动登录、角色列表、创建、选择**

在同一 `.cpp` 中加入对应方法：

```cpp
void UDBAOnlineAccountService::Register(const FDBALoginRequest& RequestData, FDBAOnLoginComplete OnComplete)
{
	Login(RequestData, OnComplete);
}

void UDBAOnlineAccountService::GuestLogin(FDBAOnLoginComplete OnComplete)
{
	FallbackLogin(OnComplete);
}

void UDBAOnlineAccountService::AutoLogin(FDBAOnLoginComplete OnComplete)
{
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame || SaveGame->SessionToken.IsEmpty())
	{
		FallbackAutoLogin(OnComplete);
		return;
	}

	SessionToken = SaveGame->SessionToken;
	TSharedRef<IHttpRequest> Request = CreateJsonRequest(TEXT("POST"), TEXT("/api/auth/refresh"), TEXT("{}"));
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, OnComplete](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		if (!bSucceeded || !HttpResponse.IsValid())
		{
			FallbackAutoLogin(OnComplete);
			return;
		}

		FDBALoginResponse Response;
		FString Error;
		if (FDBAOnlineAccountJson::ParseLoginResponse(HttpResponse->GetContentAsString(), Response, Error) && Response.bSuccess)
		{
			CacheLoginSuccess(Response);
			OnComplete.ExecuteIfBound(Response);
			return;
		}

		FDBALoginResponse Failed;
		Failed.bSuccess = false;
		Failed.ErrorMessage = TEXT("自动登录已过期");
		OnComplete.ExecuteIfBound(Failed);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::FallbackAutoLogin(FDBAOnLoginComplete OnComplete)
{
	if (OnlineConfig.bAllowMockFallback && MockService)
	{
		MockService->AutoLogin(OnComplete);
		return;
	}

	FDBALoginResponse Response;
	Response.bSuccess = false;
	Response.ErrorMessage = TEXT("需要重新登录");
	OnComplete.ExecuteIfBound(Response);
}

void UDBAOnlineAccountService::GetCharacterList(FDBAOnCharacterListLoaded OnComplete)
{
	TSharedRef<IHttpRequest> Request = CreateJsonRequest(TEXT("GET"), TEXT("/api/account/characters"), TEXT(""));
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, OnComplete](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		if (!bSucceeded || !HttpResponse.IsValid())
		{
			FallbackCharacterList(OnComplete);
			return;
		}

		TArray<FDBACharacterSummary> Characters;
		FString Error;
		if (!FDBAOnlineAccountJson::ParseCharacterListResponse(HttpResponse->GetContentAsString(), Characters, Error))
		{
			FallbackCharacterList(OnComplete);
			return;
		}
		OnComplete.ExecuteIfBound(Characters);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::FallbackCharacterList(FDBAOnCharacterListLoaded OnComplete)
{
	if (OnlineConfig.bAllowMockFallback && MockService)
	{
		MockService->GetCharacterList(OnComplete);
		return;
	}

	TArray<FDBACharacterSummary> Empty;
	OnComplete.ExecuteIfBound(Empty);
}

void UDBAOnlineAccountService::CreateCharacter(const FDBACharacterCreateRequest& RequestData, FDBAOnCharacterCreated OnComplete)
{
	const FString Body = FDBAOnlineAccountJson::BuildCreateCharacterRequest(RequestData);
	TSharedRef<IHttpRequest> Request = CreateJsonRequest(TEXT("POST"), TEXT("/api/account/characters"), Body);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, RequestData, OnComplete](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		if (!bSucceeded || !HttpResponse.IsValid())
		{
			FallbackCreateCharacter(RequestData, OnComplete);
			return;
		}

		FDBACharacterCreateResponse Response;
		FString Error;
		if (!FDBAOnlineAccountJson::ParseCreateCharacterResponse(HttpResponse->GetContentAsString(), Response, Error))
		{
			Response.bSuccess = false;
			Response.ErrorMessage = Error;
		}
		OnComplete.ExecuteIfBound(Response);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::FallbackCreateCharacter(const FDBACharacterCreateRequest& RequestData, FDBAOnCharacterCreated OnComplete)
{
	if (OnlineConfig.bAllowMockFallback && MockService)
	{
		MockService->CreateCharacter(RequestData, OnComplete);
		return;
	}

	FDBACharacterCreateResponse Response;
	Response.bSuccess = false;
	Response.ErrorMessage = TEXT("服务器不可用，无法创建角色");
	OnComplete.ExecuteIfBound(Response);
}

void UDBAOnlineAccountService::SelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete)
{
	CurrentCharacterId = CharacterId;
	Super::SelectCharacter(CharacterId, OnComplete);
}
```

- [ ] **Step 7: 运行测试和构建**

```powershell
& "$env:UE5_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.GameCore.Account.OnlineService.FallbackPolicy; Quit" -unattended -nop4 -nosplash
& "$env:UE5_ROOT\Engine\Build\BatchFiles\Build.bat" DivineBeastsArenaEditor Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex
```

预期：测试通过，编辑器构建通过。

- [ ] **Step 8: 提交**

```powershell
git add Source\GameCore\Public\GameCore\Account\DBAOnlineAccountService.h Source\GameCore\Private\GameCore\Account\DBAOnlineAccountService.cpp Source\GameCore\Private\Tests\DBAOnlineAccountServiceTests.cpp
git commit -m "feat: add online account service with mock fallback"
```

---

### Task 5: 实现登录前台流程子系统

**Files:**
- Create: `Source/GameCore/Public/GameCore/Session/DBALoginFlowSubsystem.h`
- Create: `Source/GameCore/Private/GameCore/Session/DBALoginFlowSubsystem.cpp`
- Test: `Source/GameCore/Private/Tests/DBALoginFlowSubsystemTests.cpp`

- [ ] **Step 1: 创建流程状态和测试**

创建 `Source/GameCore/Private/Tests/DBALoginFlowSubsystemTests.cpp`：

```cpp
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBALoginFlowTransitionTest,
	"DivineBeastsArena.GameCore.Session.LoginFlow.Transitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBALoginFlowTransitionTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Empty characters should require creation"), UDBALoginFlowSubsystem::ShouldEnterCharacterCreate(0));
	TestFalse(TEXT("Existing characters should not require creation"), UDBALoginFlowSubsystem::ShouldEnterCharacterCreate(1));
	return true;
}

#endif
```

- [ ] **Step 2: 运行测试确认失败**

```powershell
& "$env:UE5_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.GameCore.Session.LoginFlow.Transitions; Quit" -unattended -nop4 -nosplash
```

预期：编译失败，提示 `DBALoginFlowSubsystem.h` 不存在。

- [ ] **Step 3: 创建流程子系统头文件**

创建 `Source/GameCore/Public/GameCore/Session/DBALoginFlowSubsystem.h`：

```cpp
// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "DBALoginFlowSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDBALoginFlowState : uint8
{
	Startup,
	TryAutoLogin,
	LoginScreen,
	LoadCharacterList,
	CharacterSelect,
	CharacterCreate,
	MainLobby,
	Error
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowStateChanged, EDBALoginFlowState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLoginFlowCharactersLoaded, const TArray<FDBACharacterSummary>&, Characters);

UCLASS()
class GAMECORE_API UDBALoginFlowSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void StartLoginFlow();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitLogin(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitGuestLogin();

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitCharacterSelection(const FDBACharacterId& CharacterId);

	UFUNCTION(BlueprintCallable, Category = "DBA|LoginFlow")
	void SubmitCharacterCreation(const FDBACharacterCreateRequest& Request);

	UFUNCTION(BlueprintPure, Category = "DBA|LoginFlow")
	EDBALoginFlowState GetFlowState() const { return FlowState; }

	static bool ShouldEnterCharacterCreate(int32 CharacterCount);

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowStateChanged OnFlowStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowError OnFlowError;

	UPROPERTY(BlueprintAssignable, Category = "DBA|LoginFlow")
	FDBAOnLoginFlowCharactersLoaded OnCharactersLoaded;

protected:
	UPROPERTY()
	EDBALoginFlowState FlowState = EDBALoginFlowState::Startup;

	UPROPERTY()
	TArray<FDBACharacterSummary> CachedCharacters;

	void SetFlowState(EDBALoginFlowState NewState);
	void LoadCharactersAfterLogin();
	void EnterMainLobby();
};
```

- [ ] **Step 4: 实现流程子系统**

创建 `Source/GameCore/Private/GameCore/Session/DBALoginFlowSubsystem.cpp`：

```cpp
// Copyright FreeboozStudio. All Rights Reserved.

#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameCore/Account/DBAOnlineAccountService.h"
#include "GameCore/Account/DBAMockAccountService.h"
#include "GameCore/Session/DBAFrontendSessionSubsystem.h"

bool UDBALoginFlowSubsystem::ShouldEnterCharacterCreate(int32 CharacterCount)
{
	return CharacterCount <= 0;
}

void UDBALoginFlowSubsystem::SetFlowState(EDBALoginFlowState NewState)
{
	if (FlowState == NewState)
	{
		return;
	}
	FlowState = NewState;
	OnFlowStateChanged.Broadcast(NewState);
}

void UDBALoginFlowSubsystem::StartLoginFlow()
{
	SetFlowState(EDBALoginFlowState::TryAutoLogin);

	UDBAOnlineAccountService* AccountService = GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>();
	if (!AccountService)
	{
		SetFlowState(EDBALoginFlowState::LoginScreen);
		return;
	}

	AccountService->AutoLogin(FDBAOnLoginComplete::CreateWeakLambda(this, [this](const FDBALoginResponse& Response)
	{
		if (Response.bSuccess)
		{
			LoadCharactersAfterLogin();
			return;
		}
		SetFlowState(EDBALoginFlowState::LoginScreen);
	}));
}

void UDBALoginFlowSubsystem::SubmitLogin(const FString& Email, const FString& Password)
{
	SetFlowState(EDBALoginFlowState::TryAutoLogin);

	FDBALoginRequest Request;
	Request.LoginType = EDBALoginType::Email;
	Request.Email = Email;
	Request.Password = Password;
	Request.DeviceId = FPlatformMisc::GetDeviceId();

	UDBAOnlineAccountService* AccountService = GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>();
	if (!AccountService)
	{
		OnFlowError.Broadcast(TEXT("账号服务不可用"));
		SetFlowState(EDBALoginFlowState::LoginScreen);
		return;
	}

	AccountService->Login(Request, FDBAOnLoginComplete::CreateWeakLambda(this, [this](const FDBALoginResponse& Response)
	{
		if (Response.bSuccess)
		{
			LoadCharactersAfterLogin();
			return;
		}
		OnFlowError.Broadcast(Response.ErrorMessage);
		SetFlowState(EDBALoginFlowState::LoginScreen);
	}));
}

void UDBALoginFlowSubsystem::SubmitGuestLogin()
{
	UDBAOnlineAccountService* AccountService = GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>();
	if (!AccountService)
	{
		OnFlowError.Broadcast(TEXT("账号服务不可用"));
		return;
	}

	AccountService->GuestLogin(FDBAOnLoginComplete::CreateWeakLambda(this, [this](const FDBALoginResponse& Response)
	{
		if (Response.bSuccess)
		{
			LoadCharactersAfterLogin();
			return;
		}
		OnFlowError.Broadcast(Response.ErrorMessage);
		SetFlowState(EDBALoginFlowState::LoginScreen);
	}));
}

void UDBALoginFlowSubsystem::LoadCharactersAfterLogin()
{
	SetFlowState(EDBALoginFlowState::LoadCharacterList);

	UDBAOnlineAccountService* AccountService = GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>();
	AccountService->GetCharacterList(FDBAOnCharacterListLoaded::CreateWeakLambda(this, [this](const TArray<FDBACharacterSummary>& Characters)
	{
		CachedCharacters = Characters;
		OnCharactersLoaded.Broadcast(CachedCharacters);
		SetFlowState(ShouldEnterCharacterCreate(CachedCharacters.Num()) ? EDBALoginFlowState::CharacterCreate : EDBALoginFlowState::CharacterSelect);
	}));
}

void UDBALoginFlowSubsystem::SubmitCharacterSelection(const FDBACharacterId& CharacterId)
{
	UDBAOnlineAccountService* AccountService = GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>();
	AccountService->SelectCharacter(CharacterId, FDBAOnCharacterSelected::CreateWeakLambda(this, [this](const FDBACharacterId& SelectedId)
	{
		if (SelectedId.IsValid())
		{
			EnterMainLobby();
			return;
		}
		OnFlowError.Broadcast(TEXT("角色选择失败"));
		SetFlowState(EDBALoginFlowState::CharacterSelect);
	}));
}

void UDBALoginFlowSubsystem::SubmitCharacterCreation(const FDBACharacterCreateRequest& Request)
{
	UDBAOnlineAccountService* AccountService = GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>();
	AccountService->CreateCharacter(Request, FDBAOnCharacterCreated::CreateWeakLambda(this, [this](const FDBACharacterCreateResponse& Response)
	{
		if (Response.bSuccess)
		{
			SubmitCharacterSelection(Response.CharacterSummary.CharacterId);
			return;
		}
		OnFlowError.Broadcast(Response.ErrorMessage);
		SetFlowState(EDBALoginFlowState::CharacterCreate);
	}));
}

void UDBALoginFlowSubsystem::EnterMainLobby()
{
	if (UDBAFrontendSessionSubsystem* FrontendSession = GetGameInstance()->GetSubsystem<UDBAFrontendSessionSubsystem>())
	{
		FrontendSession->SetState(EDBAFrontendSessionState::MainLobby);
	}
	SetFlowState(EDBALoginFlowState::MainLobby);
}
```

- [ ] **Step 5: 运行测试确认通过**

```powershell
& "$env:UE5_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.GameCore.Session.LoginFlow.Transitions; Quit" -unattended -nop4 -nosplash
```

预期：测试通过。

- [ ] **Step 6: 提交**

```powershell
git add Source\GameCore\Public\GameCore\Session\DBALoginFlowSubsystem.h Source\GameCore\Private\GameCore\Session\DBALoginFlowSubsystem.cpp Source\GameCore\Private\Tests\DBALoginFlowSubsystemTests.cpp
git commit -m "feat: add frontend login flow subsystem"
```

---

### Task 6: 添加 Blueprint 可绑定登录控制器

**Files:**
- Create: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBALoginWidgetController.h`
- Create: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBALoginWidgetController.cpp`
- Create: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBACharacterSelectWidgetController.h`
- Create: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterSelectWidgetController.cpp`
- Create: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.h`
- Create: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.cpp`

- [ ] **Step 1: 创建登录控制器头文件**

创建 `UDBALoginWidgetController.h`：

```cpp
// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaWidgetControllerBase.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "UDBALoginWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBALoginUIError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBALoginUIStateChanged, EDBALoginFlowState, State);

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBALoginWidgetController : public UDBAMobaWidgetControllerBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void Start();

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void LoginWithEmail(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void LoginAsGuest();

	UPROPERTY(BlueprintAssignable, Category = "DBA|Login")
	FDBALoginUIError OnLoginError;

	UPROPERTY(BlueprintAssignable, Category = "DBA|Login")
	FDBALoginUIStateChanged OnLoginStateChanged;

protected:
	UDBALoginFlowSubsystem* GetLoginFlow() const;
};
```

- [ ] **Step 2: 实现登录控制器**

创建 `UDBALoginWidgetController.cpp`：

```cpp
// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBALoginWidgetController.h"

UDBALoginFlowSubsystem* UDBALoginWidgetController::GetLoginFlow() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>()
		: nullptr;
}

void UDBALoginWidgetController::Start()
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->OnFlowError.AddDynamic(this, &UDBALoginWidgetController::OnLoginError.Broadcast);
		Flow->OnFlowStateChanged.AddDynamic(this, &UDBALoginWidgetController::OnLoginStateChanged.Broadcast);
		Flow->StartLoginFlow();
	}
}

void UDBALoginWidgetController::LoginWithEmail(const FString& Email, const FString& Password)
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->SubmitLogin(Email, Password);
	}
}

void UDBALoginWidgetController::LoginAsGuest()
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->SubmitGuestLogin();
	}
}
```

- [ ] **Step 3: 创建角色选择控制器**

创建 `UDBACharacterSelectWidgetController.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaWidgetControllerBase.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "UDBACharacterSelectWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBACharactersChanged, const TArray<FDBACharacterSummary>&, Characters);

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterSelectWidgetController : public UDBAMobaWidgetControllerBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void BindLoginFlow();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void SelectCharacter(const FDBACharacterId& CharacterId);

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterSelect")
	FDBACharactersChanged OnCharactersChanged;
};
```

创建 `UDBACharacterSelectWidgetController.cpp`：

```cpp
#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectWidgetController.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"

void UDBACharacterSelectWidgetController::BindLoginFlow()
{
	if (UDBALoginFlowSubsystem* Flow = GetWorld()->GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>())
	{
		Flow->OnCharactersLoaded.AddDynamic(this, &UDBACharacterSelectWidgetController::OnCharactersChanged.Broadcast);
	}
}

void UDBACharacterSelectWidgetController::SelectCharacter(const FDBACharacterId& CharacterId)
{
	if (UDBALoginFlowSubsystem* Flow = GetWorld()->GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>())
	{
		Flow->SubmitCharacterSelection(CharacterId);
	}
}
```

- [ ] **Step 4: 创建角色创建控制器**

创建 `UDBACharacterCreateWidgetController.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaWidgetControllerBase.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "UDBACharacterCreateWidgetController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateWidgetController : public UDBAMobaWidgetControllerBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetCharacterName(const FString& InName);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetZodiac(EDBAZodiac InZodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetElement(EDBAElement InElement);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetFiveCamp(EDBAFiveCamp InFiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void Submit();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	FDBACharacterCreateRequest PendingRequest;
};
```

创建 `UDBACharacterCreateWidgetController.cpp`：

```cpp
#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"

void UDBACharacterCreateWidgetController::SetCharacterName(const FString& InName)
{
	PendingRequest.CharacterName = InName;
}

void UDBACharacterCreateWidgetController::SetZodiac(EDBAZodiac InZodiac)
{
	PendingRequest.DefaultZodiac = InZodiac;
}

void UDBACharacterCreateWidgetController::SetElement(EDBAElement InElement)
{
	PendingRequest.DefaultElement = InElement;
}

void UDBACharacterCreateWidgetController::SetFiveCamp(EDBAFiveCamp InFiveCamp)
{
	PendingRequest.DefaultFiveCamp = InFiveCamp;
}

void UDBACharacterCreateWidgetController::Submit()
{
	if (UDBALoginFlowSubsystem* Flow = GetWorld()->GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>())
	{
		Flow->SubmitCharacterCreation(PendingRequest);
	}
}
```

- [ ] **Step 5: 构建验证**

```powershell
& "$env:UE5_ROOT\Engine\Build\BatchFiles\Build.bat" DivineBeastsArenaEditor Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex
```

预期：构建通过，Blueprint 可看到三个控制器。

- [ ] **Step 6: 提交**

```powershell
git add Source\DivineBeastsArena\Public\GameDBA\UI\Lobby\Login Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\Login
git commit -m "feat: add login flow widget controllers"
```

---

### Task 7: 全流程验证和文档收口

**Files:**
- Modify: `Docs/superpowers/specs/2026-05-06-login-flow-design.md`
- Modify: `README.md`

- [ ] **Step 1: 运行完整自动化测试**

```powershell
& "$env:UE5_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\DivineBeastsArena.uproject" -ExecCmds="Automation RunTests DivineBeastsArena.GameCore.Account; Automation RunTests DivineBeastsArena.GameCore.Session.LoginFlow; Quit" -unattended -nop4 -nosplash
```

预期：账号 JSON、存档字段、兜底策略、登录流程测试全部通过。

- [ ] **Step 2: 运行编辑器构建**

```powershell
& "$env:UE5_ROOT\Engine\Build\BatchFiles\Build.bat" DivineBeastsArenaEditor Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex
```

预期：构建通过。

- [ ] **Step 3: 手动闭环验证**

在编辑器中启动前台地图，按以下路径验证：

```text
启动游戏
进入登录页
关闭本地 127.0.0.1:8080 后端
点击游客登录或账号登录
日志出现 [Account] Online unavailable, fallback to mock
进入角色创建
输入角色名
选择生肖 Rat
选择元素 Water
选择阵营 West
确认创建
进入主大厅
```

预期：不会卡在启动页；角色摘要包含 `FSG_Rat_Water` 和 `BAT_Rat_Water`。

- [ ] **Step 4: 更新 README 登录说明**

在 `README.md` 增加：

```markdown
## 登录闭环开发模式

登录系统第一版使用真实后端优先、Mock 兜底策略。

- 默认后端：`127.0.0.1:8080`
- 后端不可用、接口未实现或请求超时时，客户端自动切换本地 Mock 账号服务
- 账号密码错误、封禁、冻结不会进入 Mock 兜底
- 第一版闭环：启动 -> 登录 -> 角色列表 -> 创建角色 -> 大厅
```

- [ ] **Step 5: 提交**

```powershell
git add README.md Docs\superpowers\specs\2026-05-06-login-flow-design.md
git commit -m "docs: document login flow verification"
```

---

## Self-Review

Spec coverage:

- 真实后端优先：Task 3 和 Task 4 覆盖。
- Mock 兜底：Task 4 覆盖，测试包含允许与禁止兜底的错误类型。
- 启动到大厅流程：Task 5 覆盖。
- 角色创建与 v4/v4.1 数据落点：Task 2 和 Task 5 覆盖。
- Blueprint 可绑定入口：Task 6 覆盖。
- 验证标准：Task 7 覆盖。

Placeholder scan:

- 本计划没有保留未完成标记。
- 每个代码步骤都给出具体文件和代码。
- 每个验证步骤都给出命令和预期结果。

Type consistency:

- `FDBACharacterSummary` 新字段在 Task 2 定义，Task 3、Task 4、Task 5、Task 6 使用同名字段。
- 登录流状态 `EDBALoginFlowState` 在 Task 5 定义，Task 6 只引用该类型。
- 在线错误类型 `EDBAOnlineAccountError` 在 Task 3 定义，Task 4 使用同名枚举。
