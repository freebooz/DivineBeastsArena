# 07 - Boot Map、启动页与前台旅行

## 审计与迁移

| 现有实现 | 当前问题 | 收敛目标 | 迁移办法 |
| --- | --- | --- | --- |
| `GameDefaultMap` 直接为 `FrontendMap` | 缺少独立的可观测启动边界 | `L_DBA_Boot -> L_DBA_Frontend` | 现有 `FrontendMap` 暂作兼容 BootMap；创建 `L_DBA_Boot` 后仅更新 DeveloperSettings/Engine Map 配置。 |
| `UDBAGameInstance::StartLoginFlow` | 同时协调地图、UI 与流程 | 仅保留引擎级 Init/WorldChanged 转交 | 新增 `UDBAStartupCoordinatorSubsystem` 负责启动顺序。 |
| `UDBAStartupVideoWidget` 与两份 `WBP_DBA_StartupScreen` | 只有视频/跳过职责，无 ViewModel | Startup Screen 展示层 | 在原生父类新增 ViewModel 绑定与继续事件；保留已有蓝图资源。 |
| 版本检查接口 `UDBA_GameBackendConfigService::VersionCheck` | 无超时与前台回退策略 | 可选异步可达性检查 | 启动协调器设置可配置上限；失败转为 Recoverable 并继续前台。 |

## 最终链路

```text
进程 / L_DBA_Boot
  -> 配置校验
  -> 本地偏好加载
  -> 账户服务准备（不读取、不记录凭据）
  -> 可选异步版本/可达性检查 + 超时
  -> OpenLevel(L_DBA_Frontend)
  -> UDBAUILayerManagerSubsystem 创建唯一 UI Root
  -> WBP_DBA_StartupScreen
  -> 按任意键 / 点击 / 触控继续
  -> UDBAFrontendFlowSubsystem::StartLoginFlow
```

`UDBAStartupCoordinatorSubsystem` 仅加载配置中指定的 Startup Widget 软类；不会访问角色注册表、十二生肖 Mesh、角色展示 Actor 或 Niagara 资产。

## 错误策略

- 后端不可达或超时：`OfflineRecoverable`，Startup Screen 可继续进入登录页，不能黑屏或无限等待。
- Frontend Map、Flow Service、Startup Screen 配置缺失：`FatalConfiguration`，禁止启动登录流程并输出中文诊断。
- 凭据：仍由既有 Account Service 独占；Startup Coordinator 不读取、不保存、不打印 Token、RefreshToken 或密码。

## 地图资产说明

仓库当前不存在 `L_DBA_Boot.umap` / `L_DBA_Frontend.umap`，且本步骤遵守二进制资产不直改约束。因此新增了 `ADBABootGameMode` 与 `ADBABootPlayerController`，但没有伪造或改写 `.umap`。当前 `BootMap` 继续指向已有轻量前台兼容图，启动链仍由 Coordinator 接管；后续在 Editor 中创建空的 `Content/DBA/Maps/Frontend/L_DBA_Boot` 后，将 `BootMap` 更新为该资源并保存、Fix Redirectors。

## 测试契约

`DBA.Frontend.Startup.Policy` 覆盖：

- 冷启动的有效 Frontend Map 配置；
- 无网络时可恢复地进入前台；
- 配置缺失时进入 Fatal，禁止旅行。
