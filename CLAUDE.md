# DivineBeastsArena 当前进度与下一步

## 开发纲领

项目级总控提示词位于 `docs/Development/ZodiacArena_UE5_8_Codex_总控提示词.md`。继续任何 Agent 开发、审查、代码生成或验证任务前，应先以该文档作为长期指导，并结合根目录 `AGENTS.md` 的 MCP 与安全约束执行。

## 当前目标

修复上一轮审查发现的问题，重点是游戏客户端账号/角色链路，然后启动真实 Dedicated Server 和两个客户端完成联机进入游戏验证。

## 当前工作区状态

- 仓库是多应用单仓库：`DBA_GameClient` 为 UE 客户端和 Dedicated Server，`DBA_GameBackend` 为 .NET 后端，另有 Admin、Website、Launcher。
- 当前存在未提交变更，主要集中在 UE 客户端：
  - 角色展示舞台灯光和测试：`DBACharacterPresentationActor.*`、`DBACharacterPresentationStageTests.cpp`
  - 在线账号 JSON 与账号服务：`DBAOnlineAccountJson.*`、`DBAOnlineAccountService.*`
  - 账号 JSON 测试与 fallback 策略测试：`DBAOnlineAccountJsonTests.cpp`、`DBAOnlineAccountServiceTests.cpp`
  - `DBA_GameWebsite/next-env.d.ts`
- 已新增一条测试意图：`EndpointMissing` 不应再作为通用 mock fallback 条件，避免后端接口契约错误被客户端吞掉。对应文件为 `DBA_GameClient/Source/GameCore/Private/Tests/DBAOnlineAccountServiceTests.cpp`。
- 该测试刚写入，尚未完成实现修复，也尚未运行 UE 自动化测试。

## 已确认问题

1. 客户端账号注册成功后，`UDBAOnlineAccountService::Register` 会把 `Response.AccountInfo.LoginType` 强制改成 `Guest`。这会把普通注册/Email 账号误分类。
2. 客户端当前把 `EndpointMissing` 视为可 fallback 到 mock 的错误，并且角色列表 404 时会走 profile 合成角色，选择角色 404 时甚至直接视为选择成功。这会掩盖真实后端路由或契约错误。
3. 后端已有正式兼容路由：
   - `GET /api/account/characters`
   - `POST /api/account/characters`
   - `POST /api/account/characters/{characterId}/select`
   - 同时也有 `/api/players/me/characters` 兼容组
4. 因此客户端不应依赖 profile fallback 作为正常角色列表路径；fallback 只能是明确的开发/旧版本兼容策略，不能在真实后端联调里伪造成功。

## 下一步开发顺序

1. 完成客户端账号修复：
   - 从 `CanFallbackToMock` 中移除 `EndpointMissing`。
   - `GetCharacterList` 遇到 404 时返回空失败结果并记录日志，不再自动调用 profile 合成角色。
   - `SelectCharacter` 遇到 404 时返回无效角色 ID，不再把请求角色视为选择成功。
   - `Register` 保留后端解析出的 `LoginType`，不再强制改成 `Guest`。
2. 运行 UE 自动化测试，至少覆盖：
   - `DivineBeastsArena.GameCore.Account.OnlineService.FallbackPolicy`
   - `DivineBeastsArena.GameCore.Account.OnlineJson.*`
   - 登录流程相关测试
3. 确认后端可启动并通过基础链路：
   - `dotnet test DBA_GameBackend/GameBackend.sln`
   - 后端本地启动或使用既有脚本启动开发环境
4. 构建或定位真实 Dedicated Server 可执行文件，配置：
   - `UE_SERVER_EXECUTABLE_PATH`
   - `GameServerManager__AllowMockServerAllocation=false`
5. 启动独立 Dedicated Server / Worker 编排链路，验证：
   - server allocation
   - runtime register
   - ready
   - heartbeat
   - player joined / left
6. 启动两个客户端，使用不同参数避免存档和访客设备冲突：
   - `-DBASaveSlotSuffix=ClientA -DBAGuestDeviceId=ClientA`
   - `-DBASaveSlotSuffix=ClientB -DBAGuestDeviceId=ClientB`
7. 双客户端完成登录、角色创建或选择、匹配、获取连接信息、进入同一局游戏。

## 验证注意事项

- 不要把 mock fallback 当作真实联机成功。
- 如果后端接口 404、认证失败或角色选择失败，应暴露错误并修接口契约，而不是客户端伪造成功。
- `git status` 需要使用：
  ```powershell
  git -c safe.directory=D:/DivineBeastsArena status --short
  ```
- Windows PowerShell 读取中文文档时使用：
  ```powershell
  Get-Content -Raw -Encoding UTF8 <path>
  ```
