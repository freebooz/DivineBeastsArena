// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 神兽竞技场 - 主模块头文件

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * FDivineBeastsArenaModule
 * 神兽竞技场主模块
 * 负责模块的启动、关闭和全局初始化
 */
class FDivineBeastsArenaModule : public IModuleInterface
{
public:
    /**
     * 模块启动时调用
     * 用于初始化全局系统、注册资产类型、设置日志分类等
     */
    virtual void StartupModule() override;

    /**
     * 模块关闭时调用
     * 用于清理全局资源、注销系统等
     */
    virtual void ShutdownModule() override;

    /**
     * 检查模块是否支持动态重载
     * @return true 表示支持热重载
     */
    virtual bool SupportsDynamicReloading() override
    {
        return true;
    }

private:
    /**
     * 初始化日志分类
     * 在模块启动时调用，设置各子系统的日志级别
     */
    void InitializeLogging();

    /**
     * 注册资产类型
     * 在模块启动时调用，向 AssetManager 注册自定义资产类型
     */
    void RegisterAssetTypes();

    /**
     * 初始化 GameplayTag
     * 在模块启动时调用，注册项目使用的 GameplayTag
     */
    void InitializeGameplayTags();

    /**
     * 清理资源
     * 在模块关闭时调用，释放全局资源
     */
    void CleanupResources();
};