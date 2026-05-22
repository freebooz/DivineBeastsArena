// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameCore/Account/DBAAccountServiceBase.h"
#include "GameCore/Account/DBAAccountSaveGame.h"
#include "GameCore/Account/DBASaveGameVersions.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Guid.h"
#include "Misc/DateTime.h"

namespace
{
FString SanitizeSaveSlotSuffix(FString Suffix)
{
	Suffix.TrimStartAndEndInline();
	for (TCHAR& Character : Suffix)
	{
		const bool bAllowed = FChar::IsAlnum(Character) || Character == TEXT('_') || Character == TEXT('-');
		if (!bAllowed)
		{
			Character = TEXT('_');
		}
	}
	return Suffix;
}
}

UDBAAccountServiceBase::UDBAAccountServiceBase()
{
}

void UDBAAccountServiceBase::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

	LogSubsystemInfo(TEXT("账户服务初始化"));

	// 加载 Profile
	UDBAProfileSaveGame* ProfileSaveGame = LoadProfileSaveGame();
	if (ProfileSaveGame && ProfileSaveGame->IsValid())
	{
		CurrentProfile = ProfileSaveGame->PlayerProfile;
		LogSubsystemInfo(TEXT("加载 Profile 成功"));
	}
	else
	{
		LogSubsystemWarning(TEXT("加载 Profile 失败，使用默认 Profile"));
		CurrentProfile = FDBAPlayerProfile();
	}

	bIsInitialized = true;
}

void UDBAAccountServiceBase::OnSubsystemDeinitialize()
{
	LogSubsystemInfo(TEXT("账户服务反初始化"));

	// 保存 Profile
	SaveProfile();

	Super::OnSubsystemDeinitialize();
}

bool UDBAAccountServiceBase::IsSupportedInCurrentEnvironment() const
{
	// Dedicated Server 不需要账户服务
	if (GetGameInstance() && GetGameInstance()->IsDedicatedServerInstance())
	{
		return false;
	}

	return true;
}

void UDBAAccountServiceBase::Login(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Login")))
	{
		return;
	}

	LogSubsystemError(TEXT("Login - 基类未实现，派生类必须重写此方法"));

	FDBALoginResponse Response;
	Response.bSuccess = false;
	Response.ErrorMessage = TEXT("未实现");
	OnComplete.ExecuteIfBound(Response);
}

void UDBAAccountServiceBase::Register(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Register")))
	{
		return;
	}

	LogSubsystemError(TEXT("Register - 基类未实现，派生类必须重写此方法"));

	FDBALoginResponse Response;
	Response.bSuccess = false;
	Response.ErrorMessage = TEXT("未实现");
	OnComplete.ExecuteIfBound(Response);
}

void UDBAAccountServiceBase::GuestLogin(FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("GuestLogin")))
	{
		return;
	}

	LogSubsystemError(TEXT("GuestLogin - 基类未实现，派生类必须重写此方法"));

	FDBALoginResponse Response;
	Response.bSuccess = false;
	Response.ErrorMessage = TEXT("未实现");
	OnComplete.ExecuteIfBound(Response);
}

void UDBAAccountServiceBase::AutoLogin(FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("AutoLogin")))
	{
		return;
	}

	LogSubsystemError(TEXT("AutoLogin - 基类未实现，派生类必须重写此方法"));

	FDBALoginResponse Response;
	Response.bSuccess = false;
	Response.ErrorMessage = TEXT("未实现");
	OnComplete.ExecuteIfBound(Response);
}

void UDBAAccountServiceBase::Logout(FDBAOnLogoutComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Logout")))
	{
		return;
	}

	// 清空当前账户信息
	CurrentAccountInfo = FDBAAccountInfo();
	CurrentCharacterId = FDBACharacterId();
	SessionToken.Empty();
	RefreshToken.Empty();

	LogSubsystemInfo(TEXT("登出成功"));

	OnComplete.ExecuteIfBound();
}

void UDBAAccountServiceBase::GetCharacterList(FDBAOnCharacterListLoaded OnComplete)
{
	if (!EnsureGameThread(TEXT("GetCharacterList")))
	{
		return;
	}

	LogSubsystemError(TEXT("GetCharacterList - 基类未实现，派生类必须重写此方法"));

	TArray<FDBACharacterSummary> Characters;
	OnComplete.ExecuteIfBound(Characters);
}

void UDBAAccountServiceBase::GetCharacterProfile(const FDBACharacterId& CharacterId, FDBAOnCharacterProfileLoaded OnComplete)
{
	if (!EnsureGameThread(TEXT("GetCharacterProfile")))
	{
		return;
	}

	LogSubsystemError(TEXT("GetCharacterProfile - 基类未实现，派生类必须重写此方法"));

	FDBACharacterProfile Profile;
	OnComplete.ExecuteIfBound(Profile);
}

void UDBAAccountServiceBase::CreateCharacter(const FDBACharacterCreateRequest& Request, FDBAOnCharacterCreated OnComplete)
{
	if (!EnsureGameThread(TEXT("CreateCharacter")))
	{
		return;
	}

	LogSubsystemError(TEXT("CreateCharacter - 基类未实现，派生类必须重写此方法"));

	FDBACharacterCreateResponse Response;
	Response.bSuccess = false;
	Response.ErrorMessage = TEXT("未实现");
	OnComplete.ExecuteIfBound(Response);
}

void UDBAAccountServiceBase::DeleteCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterDeleted OnComplete)
{
	if (!EnsureGameThread(TEXT("DeleteCharacter")))
	{
		return;
	}

	LogSubsystemError(TEXT("DeleteCharacter - 基类未实现，派生类必须重写此方法"));

	OnComplete.ExecuteIfBound(false);
}

void UDBAAccountServiceBase::SelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete)
{
	if (!EnsureGameThread(TEXT("SelectCharacter")))
	{
		return;
	}

	CurrentCharacterId = CharacterId;

	LogSubsystemInfo(FString::Printf(TEXT("选择角色成功：%s"), *CharacterId.ToString()));

	OnComplete.ExecuteIfBound(CharacterId);
}

bool UDBAAccountServiceBase::SaveProfile()
{
	if (!EnsureGameThread(TEXT("SaveProfile")))
	{
		return false;
	}

	UDBAProfileSaveGame* ProfileSaveGame = Cast<UDBAProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UDBAProfileSaveGame::StaticClass()));
	if (!ProfileSaveGame)
	{
		LogSubsystemError(TEXT("SaveProfile - 创建 Profile 存档失败"));
		return false;
	}

	ProfileSaveGame->PlayerProfile = CurrentProfile;
	ProfileSaveGame->UpdateLastSaveTime();

	if (!SaveProfileSaveGame(ProfileSaveGame))
	{
		LogSubsystemError(TEXT("SaveProfile - 保存 Profile 存档失败"));
		return false;
	}

	LogSubsystemInfo(TEXT("保存 Profile 成功"));
	return true;
}

FString UDBAAccountServiceBase::BuildScopedSaveSlotName(const FString& BaseSlotName)
{
	FString SlotSuffix;
	if (!FParse::Value(FCommandLine::Get(), TEXT("DBASaveSlotSuffix="), SlotSuffix))
	{
		return BaseSlotName;
	}

	SlotSuffix = SanitizeSaveSlotSuffix(SlotSuffix);
	return SlotSuffix.IsEmpty() ? BaseSlotName : FString::Printf(TEXT("%s_%s"), *BaseSlotName, *SlotSuffix);
}

FDBAAccountInfo UDBAAccountServiceBase::BuildCommandLineGuestAccountInfo()
{
	FString AccountId;
	if (!FParse::Value(FCommandLine::Get(), TEXT("DBAGuestAccountId="), AccountId))
	{
		return FDBAAccountInfo();
	}

	AccountId.TrimStartAndEndInline();
	if (AccountId.IsEmpty())
	{
		return FDBAAccountInfo();
	}

	FString DisplayName;
	FParse::Value(FCommandLine::Get(), TEXT("DBAGuestDisplayName="), DisplayName);
	DisplayName.TrimStartAndEndInline();
	if (DisplayName.IsEmpty())
	{
		DisplayName = AccountId;
	}

	FDBAAccountInfo AccountInfo;
	AccountInfo.AccountId = FDBAAccountId(AccountId);
	AccountInfo.DisplayName = DisplayName;
	AccountInfo.LoginType = EDBALoginType::Guest;
	AccountInfo.Status = EDBAAccountStatus::Normal;
	AccountInfo.Level = 1;
	AccountInfo.Experience = 0;
	AccountInfo.CreateTime = FDateTime::UtcNow().ToUnixTimestamp();
	AccountInfo.LastLoginTime = AccountInfo.CreateTime;
	return AccountInfo;
}

UDBAAccountSaveGame* UDBAAccountServiceBase::LoadAccountSaveGame()
{
	const FString SlotName = BuildScopedSaveSlotName(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT);

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		LogSubsystemInfo(FString::Printf(TEXT("LoadAccountSaveGame - 存档不存在：%s"), *SlotName));
		return nullptr;
	}

	USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	UDBAAccountSaveGame* AccountSaveGame = Cast<UDBAAccountSaveGame>(SaveGameObject);

	if (!AccountSaveGame)
	{
		LogSubsystemError(FString::Printf(TEXT("LoadAccountSaveGame - 加载存档失败：%s"), *SlotName));
		HandleCorruptedSaveGame(SlotName);
		return nullptr;
	}

	if (!AccountSaveGame->IsVersionCompatible())
	{
		LogSubsystemError(FString::Printf(TEXT("LoadAccountSaveGame - 存档版本不兼容：%s"), *SlotName));
		return nullptr;
	}

	if (AccountSaveGame->NeedsMigration())
	{
		LogSubsystemWarning(FString::Printf(TEXT("LoadAccountSaveGame - 存档需要迁移：%s"), *SlotName));
		if (!AccountSaveGame->MigrateToCurrentVersion())
		{
			LogSubsystemError(FString::Printf(TEXT("LoadAccountSaveGame - 存档迁移失败：%s"), *SlotName));
			return nullptr;
		}
		SaveAccountSaveGame(AccountSaveGame);
	}

	LogSubsystemInfo(FString::Printf(TEXT("LoadAccountSaveGame - 加载存档成功：%s"), *SlotName));
	return AccountSaveGame;
}

bool UDBAAccountServiceBase::SaveAccountSaveGame(UDBAAccountSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		LogSubsystemError(TEXT("SaveAccountSaveGame - 存档为空"));
		return false;
	}

	const FString SlotName = BuildScopedSaveSlotName(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT);

	SaveGame->UpdateLastSaveTime();

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0))
	{
		LogSubsystemError(FString::Printf(TEXT("SaveAccountSaveGame - 保存存档失败：%s"), *SlotName));
		return false;
	}

	LogSubsystemInfo(FString::Printf(TEXT("SaveAccountSaveGame - 保存存档成功：%s"), *SlotName));
	return true;
}

UDBAProfileSaveGame* UDBAAccountServiceBase::LoadProfileSaveGame()
{
	const FString SlotName = BuildScopedSaveSlotName(DBASaveGameVersions::SlotNames::PROFILE_SLOT);

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		LogSubsystemInfo(FString::Printf(TEXT("LoadProfileSaveGame - 存档不存在：%s"), *SlotName));
		return nullptr;
	}

	USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	UDBAProfileSaveGame* ProfileSaveGame = Cast<UDBAProfileSaveGame>(SaveGameObject);

	if (!ProfileSaveGame)
	{
		LogSubsystemError(FString::Printf(TEXT("LoadProfileSaveGame - 加载存档失败：%s"), *SlotName));
		HandleCorruptedSaveGame(SlotName);
		return nullptr;
	}

	if (!ProfileSaveGame->IsVersionCompatible())
	{
		LogSubsystemError(FString::Printf(TEXT("LoadProfileSaveGame - 存档版本不兼容：%s"), *SlotName));
		return nullptr;
	}

	if (ProfileSaveGame->NeedsMigration())
	{
		LogSubsystemWarning(FString::Printf(TEXT("LoadProfileSaveGame - 存档需要迁移：%s"), *SlotName));
		if (!ProfileSaveGame->MigrateToCurrentVersion())
		{
			LogSubsystemError(FString::Printf(TEXT("LoadProfileSaveGame - 存档迁移失败：%s"), *SlotName));
			return nullptr;
		}
		SaveProfileSaveGame(ProfileSaveGame);
	}

	LogSubsystemInfo(FString::Printf(TEXT("LoadProfileSaveGame - 加载存档成功：%s"), *SlotName));
	return ProfileSaveGame;
}

bool UDBAAccountServiceBase::SaveProfileSaveGame(UDBAProfileSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		LogSubsystemError(TEXT("SaveProfileSaveGame - 存档为空"));
		return false;
	}

	const FString SlotName = BuildScopedSaveSlotName(DBASaveGameVersions::SlotNames::PROFILE_SLOT);

	SaveGame->UpdateLastSaveTime();

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0))
	{
		LogSubsystemError(FString::Printf(TEXT("SaveProfileSaveGame - 保存存档失败：%s"), *SlotName));
		return false;
	}

	LogSubsystemInfo(FString::Printf(TEXT("SaveProfileSaveGame - 保存存档成功：%s"), *SlotName));
	return true;
}

UDBAAccountSaveGame* UDBAAccountServiceBase::CreateDefaultAccountSaveGame()
{
	UDBAAccountSaveGame* SaveGame = Cast<UDBAAccountSaveGame>(UGameplayStatics::CreateSaveGameObject(UDBAAccountSaveGame::StaticClass()));
	if (!SaveGame)
	{
		LogSubsystemError(TEXT("CreateDefaultAccountSaveGame - 创建存档失败"));
		return nullptr;
	}

	SaveGame->ResetToDefault();
	LogSubsystemInfo(TEXT("CreateDefaultAccountSaveGame - 创建默认存档成功"));
	return SaveGame;
}

UDBAProfileSaveGame* UDBAAccountServiceBase::CreateDefaultProfileSaveGame()
{
	UDBAProfileSaveGame* SaveGame = Cast<UDBAProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UDBAProfileSaveGame::StaticClass()));
	if (!SaveGame)
	{
		LogSubsystemError(TEXT("CreateDefaultProfileSaveGame - 创建存档失败"));
		return nullptr;
	}

	SaveGame->ResetToDefault();
	LogSubsystemInfo(TEXT("CreateDefaultProfileSaveGame - 创建默认存档成功"));
	return SaveGame;
}

bool UDBAAccountServiceBase::HandleCorruptedSaveGame(const FString& SlotName)
{
	LogSubsystemError(FString::Printf(TEXT("HandleCorruptedSaveGame - 存档损坏：%s"), *SlotName));

	// 尝试加载备份
	const FString BackupSlotName = SlotName + DBASaveGameVersions::SlotNames::BACKUP_SUFFIX;
	if (UGameplayStatics::DoesSaveGameExist(BackupSlotName, 0))
	{
		LogSubsystemInfo(FString::Printf(TEXT("HandleCorruptedSaveGame - 尝试加载备份：%s"), *BackupSlotName));

		USaveGame* BackupSaveGame = UGameplayStatics::LoadGameFromSlot(BackupSlotName, 0);
		if (BackupSaveGame)
		{
			// 恢复备份到主存档
			if (UGameplayStatics::SaveGameToSlot(BackupSaveGame, SlotName, 0))
			{
				LogSubsystemInfo(FString::Printf(TEXT("HandleCorruptedSaveGame - 从备份恢复成功：%s"), *SlotName));
				return true;
			}
		}
	}

	LogSubsystemError(FString::Printf(TEXT("HandleCorruptedSaveGame - 无法修复存档：%s"), *SlotName));
	return false;
}

FDBAAccountId UDBAAccountServiceBase::GenerateGuestAccountId()
{
	const FString GuestId = FString::Printf(TEXT("Guest_%s"), *FGuid::NewGuid().ToString());
	return FDBAAccountId(GuestId);
}

FDBACharacterId UDBAAccountServiceBase::GenerateCharacterId()
{
	const FString CharId = FGuid::NewGuid().ToString();
	return FDBACharacterId(CharId);
}

bool UDBAAccountServiceBase::ValidateCharacterName(const FString& CharacterName, FString& OutErrorMessage)
{
	if (CharacterName.Len() < 2)
	{
		OutErrorMessage = TEXT("角色名称长度需为 2-16 个字符");
		return false;
	}

	for (int32 Index = 0; Index < CharacterName.Len(); ++Index)
	{
		const TCHAR Character = CharacterName[Index];
		const bool bIsChinese = Character >= 0x4e00 && Character <= 0x9fa5;
		if (!FChar::IsAlnum(Character) && Character != TEXT('_') && !bIsChinese)
		{
			OutErrorMessage = TEXT("角色名称只允许中文、英文、数字和下划线");
			return false;
		}
	}

	if (CharacterName.IsEmpty())
	{
		OutErrorMessage = TEXT("角色名称不能为空");
		return false;
	}

	if (CharacterName.Len() > 16)
	{
		OutErrorMessage = TEXT("角色名称最多 16 个字符");
		return false;
	}

	// 检查非法字符
	const FString IllegalChars = TEXT("<>|\\/:*?\"");
	for (int32 i = 0; i < IllegalChars.Len(); ++i)
	{
		if (CharacterName.Contains(IllegalChars.Mid(i, 1)))
		{
			OutErrorMessage = TEXT("角色名称包含非法字符");
			return false;
		}
	}

	return true;
}
