# 06 - 前台 UI 根布局与分层架构

## 审计结论与迁移办法

| 现有实现 | 当前引用情况 | 目标实现 | 迁移办法 |
| --- | --- | --- | --- |
| `UDBAGameUIManager` 以不同 ZOrder 直接 `AddToViewport` | 启动、登录、角色、大厅及局内 UI 均在使用 | 保留为兼容门面 | 所有挂载改为委托 `UDBAUILayerManagerSubsystem`；不再允许其直接加入业务 Widget 到视口。 |
| `WBP_DBA_UIRootLayout` | 存在于多个历史 Content 目录，父类不一致 | `UDBAUIRootLayout` 为唯一运行时根布局 | 本步骤不修改二进制 UMG；后续资产步骤将保留的表现资产重新父类化到 C++ 根布局并 Fix Redirectors。 |
| `WBP_DBA_ModalDialog`、`WBP_DBA_SystemToast` | 历史 UMG 资源已存在，但无稳定 C++ 父类 | `UDBAModalDialogWidgetBase`、`UDBASystemToastWidgetBase` | 新资源只能继承 C++ 基类；旧资源先由 Layer Manager 作为普通 Widget 安全挂载。 |
| `UDBALoadingScreenWidgetBase` | 大厅进入流程在使用 | 局部大厅 Loading 保留；全局异步覆盖使用 `UDBAGlobalLoadingWidgetBase` | 新跨页异步请求必须使用 Begin/EndGlobalLoading 的请求令牌。 |

## 唯一所有权

`UDBAUILayerManagerSubsystem` 是唯一允许把 `UDBAUIRootLayout` 加入 Viewport 的对象。业务界面只能由 `UDBAGameUIManager`（旧入口）或后续 Flow UI Controller 调用 `MountWidget`。

```text
Flow / WidgetController / Subsystem
                |
                v
 UDBAUILayerManagerSubsystem
                |
                v
 UDBAUIRootLayout (唯一 AddToViewport)
   ├── BackgroundLayer
   ├── ScreenLayer
   ├── ModalLayer
   ├── ToastLayer
   ├── TooltipLayer
   └── DebugLayer
```

- `BackgroundLayer`：持久背景与非交互场景 UI。
- `ScreenLayer`：唯一活动业务 Screen；替换 Screen 不影响 Modal。
- `ModalLayer`：可叠加的阻塞对话框与全局 Loading。
- `ToastLayer`：Toast、ErrorBanner、NetworkStatus。
- `TooltipLayer`：短暂提示，不参与流程导航。
- `DebugLayer`：大厅 HUD、交互提示、调试展示；不应承载业务流程页面。

## 边界

- Widget：布局、动画、输入；不得 `AddToViewport`、直接发 HTTP 或改变前台业务状态。
- ViewModel：可绑定显示状态；不保存 Token 或权威角色数据。
- WidgetController：将输入意图交给 Flow/Subsystem，并监听状态事件。
- Subsystem：生命周期、异步请求、层挂载、会话与业务协调。

## 输入与返回

`UDBACommonScreenBase` 统一处理 Escape 与 `Gamepad_Special_Right`，并广播 Back 意图；Android 的 CommonInput 返回映射应路由到相同入口。`UDBAUILayerManagerSubsystem::HandleBackAction` 优先处理最上层 Modal。业务 Screen 的 Back 必须交给 Frontend Flow 状态机，不能由 Widget 自行跳转或销毁其他页面。

## 全局 Loading

`BeginGlobalLoading` 返回不可预测的 `FName` 请求令牌。每一个异步请求只能结束自己的令牌；仅当令牌集合变空时才移除 Global Loading。因此嵌套请求不会提前关闭覆盖层。

## Dedicated Server

Layer Manager 在 Dedicated Server 或无 World 时拒绝创建 RootLayout、Widget 与输入模式。项目仍保留现有模块构建边界；本步骤没有将 CommonUI/UMG 运行时对象创建到服务器路径。

## 后续资产前置

1. 在 Editor 中将保留的 `WBP_DBA_UIRootLayout`、`WBP_DBA_ModalDialog`、`WBP_DBA_SystemToast` 迁移到 `Content/DBA/UI/Common`，并以对应 C++ 基类重新父类化。
2. 创建 `WBP_DBA_GlobalLoading`、`WBP_DBA_ErrorBanner`、`WBP_DBA_NetworkStatus`，仅承载布局与动画。
3. 保存并 Fix Redirectors 后，逐步删除重复的历史 UI 资产目录；本步骤不删除任何二进制资产。
