// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Flow/DBAFrontendStateMachine.h"

namespace DBAFrontendStateMachine
{
	bool IsCharacterCreationState(const EDBAFrontendState State)
	{
		return State == EDBAFrontendState::CharacterCreate_Zodiac
			|| State == EDBAFrontendState::CharacterCreate_Element
			|| State == EDBAFrontendState::CharacterCreate_FiveCamp
			|| State == EDBAFrontendState::CharacterCreate_Confirm;
	}

	bool CanTransition(const EDBAFrontendState From, const EDBAFrontendState To)
	{
		if (From == To)
		{
			return true;
		}

		switch (From)
		{
		case EDBAFrontendState::Bootstrapping:
			return To == EDBAFrontendState::Startup || To == EDBAFrontendState::FatalError;
		case EDBAFrontendState::Startup:
			return To == EDBAFrontendState::AutoLogin || To == EDBAFrontendState::Login || To == EDBAFrontendState::FatalError;
		case EDBAFrontendState::AutoLogin:
			return To == EDBAFrontendState::ServerSelect || To == EDBAFrontendState::Login || To == EDBAFrontendState::RecoverableError || To == EDBAFrontendState::FatalError;
		case EDBAFrontendState::Login:
			return To == EDBAFrontendState::Register || To == EDBAFrontendState::ServerSelect || To == EDBAFrontendState::CharacterRosterLoading || To == EDBAFrontendState::RecoverableError || To == EDBAFrontendState::FatalError;
		case EDBAFrontendState::Register:
			return To == EDBAFrontendState::Login || To == EDBAFrontendState::ServerSelect || To == EDBAFrontendState::RecoverableError || To == EDBAFrontendState::FatalError;
		case EDBAFrontendState::ServerSelect:
			return To == EDBAFrontendState::Login || To == EDBAFrontendState::CharacterRosterLoading || To == EDBAFrontendState::RecoverableError;
		case EDBAFrontendState::CharacterRosterLoading:
			return To == EDBAFrontendState::CharacterSelect || To == EDBAFrontendState::CharacterCreate_Zodiac || To == EDBAFrontendState::ServerSelect || To == EDBAFrontendState::Login || To == EDBAFrontendState::RecoverableError || To == EDBAFrontendState::FatalError;
		case EDBAFrontendState::CharacterSelect:
			return To == EDBAFrontendState::CharacterRosterLoading || To == EDBAFrontendState::CharacterCreate_Zodiac || To == EDBAFrontendState::ServerSelect || To == EDBAFrontendState::EnteringWorld || To == EDBAFrontendState::Login || To == EDBAFrontendState::RecoverableError;
		case EDBAFrontendState::CharacterCreate_Zodiac:
			return To == EDBAFrontendState::CharacterCreate_Element || To == EDBAFrontendState::CharacterSelect || To == EDBAFrontendState::RecoverableError;
		case EDBAFrontendState::CharacterCreate_Element:
			return To == EDBAFrontendState::CharacterCreate_Zodiac || To == EDBAFrontendState::CharacterCreate_FiveCamp || To == EDBAFrontendState::CharacterSelect || To == EDBAFrontendState::RecoverableError;
		case EDBAFrontendState::CharacterCreate_FiveCamp:
			return To == EDBAFrontendState::CharacterCreate_Element || To == EDBAFrontendState::CharacterCreate_Confirm || To == EDBAFrontendState::CharacterSelect || To == EDBAFrontendState::RecoverableError;
		case EDBAFrontendState::CharacterCreate_Confirm:
			return To == EDBAFrontendState::CharacterCreate_FiveCamp || To == EDBAFrontendState::EnteringWorld || To == EDBAFrontendState::CharacterSelect || To == EDBAFrontendState::RecoverableError;
		case EDBAFrontendState::EnteringWorld:
			return To == EDBAFrontendState::CharacterSelect || To == EDBAFrontendState::RecoverableError || To == EDBAFrontendState::FatalError;
		case EDBAFrontendState::RecoverableError:
			// 可恢复错误必须能回到原角色创建步骤，避免网络短暂中断强制丢失已验证的 Draft；
			// 全局维护也允许回启动页，仍由 Flow 决定是否需要清理会话。
			return To == EDBAFrontendState::Startup || To == EDBAFrontendState::Login || To == EDBAFrontendState::Register
				|| To == EDBAFrontendState::ServerSelect || To == EDBAFrontendState::CharacterSelect
				|| To == EDBAFrontendState::CharacterCreate_Zodiac || To == EDBAFrontendState::CharacterCreate_Element
				|| To == EDBAFrontendState::CharacterCreate_FiveCamp || To == EDBAFrontendState::CharacterCreate_Confirm
				|| To == EDBAFrontendState::FatalError;
		case EDBAFrontendState::FatalError:
			return To == EDBAFrontendState::Bootstrapping;
		default:
			return false;
		}
	}
}
