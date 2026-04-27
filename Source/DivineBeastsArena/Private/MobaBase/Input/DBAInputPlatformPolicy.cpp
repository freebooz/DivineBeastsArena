// Copyright FreeboozStudio. All Rights Reserved.

#include "MobaBase/Input/DBAInputPlatformPolicy.h"

EDBAInputPlatform UDBAInputPlatformPolicy::GetCurrentPlatform_Implementation() const
{
	// 默认实现：根据编译平台返回
#if PLATFORM_ANDROID
	return EDBAInputPlatform::Android;
#else
	return EDBAInputPlatform::PC;
#endif
}

bool UDBAInputPlatformPolicy::IsTouchPlatform() const
{
	return GetCurrentPlatform() == EDBAInputPlatform::Android;
}
