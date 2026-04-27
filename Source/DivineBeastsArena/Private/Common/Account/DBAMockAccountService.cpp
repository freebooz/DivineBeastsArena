// Copyright FreeboozStudio. All Rights Reserved.

#include "Common/Account/DBAMockAccountService.h"
#include "Common/Account/DBAAccountSaveGame.h"
#include "Misc/DateTime.h"

UDBAMockAccountService::UDBAMockAccountService()
{
}

void UDBAMockAccountService::Login(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Login")))
	{
		return;
	}

	LogSubsystemInfo(TEXT("Login - Mock 账户服务只支持 Guest 登录"));

	FDBALoginResponse Response;
	Response.bSuccess = false;
	Response.ErrorMessage = TEXT("Mock 账户服务只支持 Guest 登录");
	OnComplete.ExecuteIfBound(Response);
}

void UDBAMockAccountService::Register(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Register")))
	{
		return;
	}

	LogSubsystemInfo(TEXT("Register - Mock 账户服务只支持 Guest 登录"));

	FDBALoginResponse Response;
	Response.bSuccess = false;
	Response.ErrorMessage = TEXT("Mock 账户服务只支持 Guest 登录");
	OnComplete.ExecuteIfBound(Response);
}

void UDBAMockAccountService::GuestLogin(FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("GuestLogin")))
	{
		return;
	}

	LogSubsystemInfo(TEXT("GuestLogin - 开始 Guest 登录"));

	PerformGuestLogin(true, OnComplete);
}

void UDBAMockAccountService::AutoLogin(FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("AutoLogin")))
	{
		return;
	}

	LogSubsystemInfo(TEXT("AutoLogin - 尝试自动登录"));

	// 尝试加载本地存档
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (SaveGame && SaveGame->IsValid())
	{
		// 使用存档数据登录
		CurrentAccountInfo = SaveGame->AccountInfo;
		CurrentCharacterId = SaveGame->CurrentCharacterId;
		SessionToken = FGuid::NewGuid().ToString();

		// 更新最后登录时间
		CurrentAccountInfo.LastLoginTime = FDateTime::UtcNow().ToUnixTimestamp();
		SaveGame->AccountInfo = CurrentAccountInfo;
		SaveAccountSaveGame(SaveGame);

		LogSubsystemInfo(FString::Printf(TEXT("AutoLogin - 自动登录成功：%s"), *CurrentAccountInfo.DisplayName));

		FDBALoginResponse Response;
		Response.bSuccess = true;
		Response.AccountInfo = CurrentAccountInfo;
		Response.SessionToken = SessionToken;
		OnComplete.ExecuteIfBound(Response);
	}
	else
	{
		// 没有本地存档，执行 Guest 登录
		LogSubsystemInfo(TEXT("AutoLogin - 没有本地存档，执行 Guest 登录"));
		PerformGuestLogin(true, OnComplete);
	}
}

void UDBAMockAccountService::Logout(FDBAOnLogoutComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Logout")))
	{
		return;
	}

	LogSubsystemInfo(TEXT("Logout - 开始登出"));

	// 保存当前账户
	SaveCurrentAccount();

	// 调用基类登出
	Super::Logout(OnComplete);
}

void UDBAMockAccountService::GetCharacterList(FDBAOnCharacterListLoaded OnComplete)
{
	if (!EnsureGameThread(TEXT("GetCharacterList")))
	{
		return;
	}

	if (!IsLoggedIn())
	{
		LogSubsystemError(TEXT("GetCharacterList - 未登录"));
		TArray<FDBACharacterSummary> EmptyList;
		OnComplete.ExecuteIfBound(EmptyList);
		return;
	}

	// 从存档加载角色列表
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (SaveGame && SaveGame->IsValid())
	{
		LogSubsystemInfo(FString::Printf(TEXT("GetCharacterList - 加载角色列表成功，数量：%d"), SaveGame->Characters.Num()));
		OnComplete.ExecuteIfBound(SaveGame->Characters);
	}
	else
	{
		LogSubsystemWarning(TEXT("GetCharacterList - 加载角色列表失败"));
		TArray<FDBACharacterSummary> EmptyList;
		OnComplete.ExecuteIfBound(EmptyList);
	}
}

void UDBAMockAccountService::GetCharacterProfile(const FDBACharacterId& CharacterId, FDBAOnCharacterProfileLoaded OnComplete)
{
	if (!EnsureGameThread(TEXT("GetCharacterProfile")))
	{
		return;
	}

	if (!IsLoggedIn())
	{
		LogSubsystemError(TEXT("GetCharacterProfile - 未登录"));
		FDBACharacterProfile EmptyProfile;
		OnComplete.ExecuteIfBound(EmptyProfile);
		return;
	}

	// 从缓存加载角色详细信息
	FDBACharacterProfile Profile = LoadCharacterProfileFromCache(CharacterId);
	if (Profile.IsValid())
	{
		LogSubsystemInfo(FString::Printf(TEXT("GetCharacterProfile - 加载角色详细信息成功：%s"), *Profile.Summary.CharacterName));
		OnComplete.ExecuteIfBound(Profile);
	}
	else
	{
		LogSubsystemWarning(FString::Printf(TEXT("GetCharacterProfile - 角色不存在：%s"), *CharacterId.ToString()));
		FDBACharacterProfile EmptyProfile;
		OnComplete.ExecuteIfBound(EmptyProfile);
	}
}

void UDBAMockAccountService::CreateCharacter(const FDBACharacterCreateRequest& Request, FDBAOnCharacterCreated OnComplete)
{
	if (!EnsureGameThread(TEXT("CreateCharacter")))
	{
		return;
	}

	if (!IsLoggedIn())
	{
		LogSubsystemError(TEXT("CreateCharacter - 未登录"));
		FDBACharacterCreateResponse Response;
		Response.bSuccess = false;
		Response.ErrorMessage = TEXT("未登录");
		OnComplete.ExecuteIfBound(Response);
		return;
	}

	// 验证角色名称
	FString ErrorMessage;
	if (!ValidateCharacterName(Request.CharacterName, ErrorMessage))
	{
		LogSubsystemError(FString::Printf(TEXT("CreateCharacter - 角色名称无效：%s"), *ErrorMessage));
		FDBACharacterCreateResponse Response;
		Response.bSuccess = false;
		Response.ErrorMessage = ErrorMessage;
		OnComplete.ExecuteIfBound(Response);
		return;
	}

	// 加载存档
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame)
	{
		SaveGame = CreateDefaultAccountSaveGame();
		if (!SaveGame)
		{
			LogSubsystemError(TEXT("CreateCharacter - 创建存档失败"));
			FDBACharacterCreateResponse Response;
			Response.bSuccess = false;
			Response.ErrorMessage = TEXT("创建存档失败");
			OnComplete.ExecuteIfBound(Response);
			return;
		}
		SaveGame->AccountInfo = CurrentAccountInfo;
	}

	// 检查角色数量限制
	const int32 MaxCharacters = 5;
	if (SaveGame->Characters.Num() >= MaxCharacters)
	{
		LogSubsystemError(FString::Printf(TEXT("CreateCharacter - 角色数量已达上限：%d"), MaxCharacters));
		FDBACharacterCreateResponse Response;
		Response.bSuccess = false;
		Response.ErrorMessage = FString::Printf(TEXT("角色数量已达上限：%d"), MaxCharacters);
		OnComplete.ExecuteIfBound(Response);
		return;
	}

	// 创建角色摘要
	FDBACharacterSummary Summary;
	Summary.CharacterId = GenerateCharacterId();
	Summary.CharacterName = Request.CharacterName;
	Summary.DefaultZodiac = Request.DefaultZodiac;
	Summary.DefaultElement = Request.DefaultElement;
	Summary.DefaultFiveCamp = Request.DefaultFiveCamp;
	Summary.Level = 1;
	Summary.CreateTime = FDateTime::UtcNow().ToUnixTimestamp();
	Summary.LastUsedTime = Summary.CreateTime;

	// 添加到存档
	if (!SaveGame->AddCharacter(Summary))
	{
		LogSubsystemError(TEXT("CreateCharacter - 添加角色到存档失败"));
		FDBACharacterCreateResponse Response;
		Response.bSuccess = false;
		Response.ErrorMessage = TEXT("添加角色到存档失败");
		OnComplete.ExecuteIfBound(Response);
		return;
	}

	// 创建角色详细信息
	FDBACharacterProfile Profile;
	Profile.Summary = Summary;
	Profile.TotalGames = 0;
	Profile.WinGames = 0;
	Profile.TotalKills = 0;
	Profile.TotalDeaths = 0;
	Profile.TotalAssists = 0;

	// 保存到缓存
	SaveCharacterProfileToCache(Profile);

	// 保存存档
	if (!SaveAccountSaveGame(SaveGame))
	{
		LogSubsystemError(TEXT("CreateCharacter - 保存存档失败"));
		FDBACharacterCreateResponse Response;
		Response.bSuccess = false;
		Response.ErrorMessage = TEXT("保存存档失败");
		OnComplete.ExecuteIfBound(Response);
		return;
	}

	LogSubsystemInfo(FString::Printf(TEXT("CreateCharacter - 创建角色成功：%s"), *Summary.CharacterName));

	FDBACharacterCreateResponse Response;
	Response.bSuccess = true;
	Response.CharacterSummary = Summary;
	OnComplete.ExecuteIfBound(Response);
}

void UDBAMockAccountService::DeleteCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterDeleted OnComplete)
{
	if (!EnsureGameThread(TEXT("DeleteCharacter")))
	{
		return;
	}

	if (!IsLoggedIn())
	{
		LogSubsystemError(TEXT("DeleteCharacter - 未登录"));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	// 加载存档
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame)
	{
		LogSubsystemError(TEXT("DeleteCharacter - 加载存档失败"));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	// 删除角色
	if (!SaveGame->RemoveCharacter(CharacterId))
	{
		LogSubsystemError(FString::Printf(TEXT("DeleteCharacter - 删除角色失败：%s"), *CharacterId.ToString()));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	// 从缓存删除
	CharacterProfileCache.Remove(CharacterId);

	// 保存存档
	if (!SaveAccountSaveGame(SaveGame))
	{
		LogSubsystemError(TEXT("DeleteCharacter - 保存存档失败"));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	LogSubsystemInfo(FString::Printf(TEXT("DeleteCharacter - 删除角色成功：%s"), *CharacterId.ToString()));
	OnComplete.ExecuteIfBound(true);
}

void UDBAMockAccountService::SelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete)
{
	if (!EnsureGameThread(TEXT("SelectCharacter")))
	{
		return;
	}

	if (!IsLoggedIn())
	{
		LogSubsystemError(TEXT("SelectCharacter - 未登录"));
		OnComplete.ExecuteIfBound(FDBACharacterId());
		return;
	}

	// 加载存档
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame)
	{
		LogSubsystemError(TEXT("SelectCharacter - 加载存档失败"));
		OnComplete.ExecuteIfBound(FDBACharacterId());
		return;
	}

	// 检查角色是否存在
	const FDBACharacterSummary* Character = SaveGame->FindCharacter(CharacterId);
	if (!Character)
	{
		LogSubsystemError(FString::Printf(TEXT("SelectCharacter - 角色不存在：%s"), *CharacterId.ToString()));
		OnComplete.ExecuteIfBound(FDBACharacterId());
		return;
	}

	// 更新当前角色
	SaveGame->CurrentCharacterId = CharacterId;

	// 保存存档
	if (!SaveAccountSaveGame(SaveGame))
	{
		LogSubsystemError(TEXT("SelectCharacter - 保存存档失败"));
		OnComplete.ExecuteIfBound(FDBACharacterId());
		return;
	}

	// 调用基类选择角色
	Super::SelectCharacter(CharacterId, OnComplete);
}

void UDBAMockAccountService::PerformGuestLogin(bool bCreateNew, FDBAOnLoginComplete OnComplete)
{
	FDBALoginResponse Response;

	if (bCreateNew)
	{
		// 创建新 Guest 账户
		CurrentAccountInfo = CreateNewGuestAccount();
		SessionToken = FGuid::NewGuid().ToString();

		// 保存到存档
		if (!SaveCurrentAccount())
		{
			LogSubsystemError(TEXT("PerformGuestLogin - 保存账户失败"));
			Response.bSuccess = false;
			Response.ErrorMessage = TEXT("保存账户失败");
			OnComplete.ExecuteIfBound(Response);
			return;
		}

		LogSubsystemInfo(FString::Printf(TEXT("PerformGuestLogin - Guest 登录成功：%s"), *CurrentAccountInfo.DisplayName));

		Response.bSuccess = true;
		Response.AccountInfo = CurrentAccountInfo;
		Response.SessionToken = SessionToken;
		OnComplete.ExecuteIfBound(Response);
	}
	else
	{
		LogSubsystemError(TEXT("PerformGuestLogin - 不支持的登录模式"));
		Response.bSuccess = false;
		Response.ErrorMessage = TEXT("不支持的登录模式");
		OnComplete.ExecuteIfBound(Response);
	}
}

FDBAAccountInfo UDBAMockAccountService::CreateNewGuestAccount()
{
	FDBAAccountInfo AccountInfo;
	AccountInfo.AccountId = GenerateGuestAccountId();
	AccountInfo.DisplayName = FString::Printf(TEXT("游客_%s"), *FGuid::NewGuid().ToString().Left(8));
	AccountInfo.LoginType = EDBALoginType::Guest;
	AccountInfo.Status = EDBAAccountStatus::Normal;
	AccountInfo.Level = 1;
	AccountInfo.Experience = 0;
	AccountInfo.CreateTime = FDateTime::UtcNow().ToUnixTimestamp();
	AccountInfo.LastLoginTime = AccountInfo.CreateTime;

	return AccountInfo;
}

bool UDBAMockAccountService::SaveCurrentAccount()
{
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame)
	{
		SaveGame = CreateDefaultAccountSaveGame();
		if (!SaveGame)
		{
			LogSubsystemError(TEXT("SaveCurrentAccount - 创建存档失败"));
			return false;
		}
	}

	SaveGame->AccountInfo = CurrentAccountInfo;
	SaveGame->CurrentCharacterId = CurrentCharacterId;

	return SaveAccountSaveGame(SaveGame);
}

FDBACharacterProfile UDBAMockAccountService::LoadCharacterProfileFromCache(const FDBACharacterId& CharacterId)
{
	// 先从缓存查找
	if (CharacterProfileCache.Contains(CharacterId))
	{
		return CharacterProfileCache[CharacterId];
	}

	// 从存档加载
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame)
	{
		return FDBACharacterProfile();
	}

	const FDBACharacterSummary* Summary = SaveGame->FindCharacter(CharacterId);
	if (!Summary)
	{
		return FDBACharacterProfile();
	}

	// 创建默认 Profile
	FDBACharacterProfile Profile;
	Profile.Summary = *Summary;
	Profile.TotalGames = 0;
	Profile.WinGames = 0;
	Profile.TotalKills = 0;
	Profile.TotalDeaths = 0;
	Profile.TotalAssists = 0;

	// 保存到缓存
	CharacterProfileCache.Add(CharacterId, Profile);

	return Profile;
}

void UDBAMockAccountService::SaveCharacterProfileToCache(const FDBACharacterProfile& Profile)
{
	if (!Profile.IsValid())
	{
		return;
	}

	CharacterProfileCache.Add(Profile.Summary.CharacterId, Profile);
}
