# 步骤29修改与人工审核启动报告

## 修改结论

启动到角色创建链已经收敛为 `UDBAFrontendFlowSubsystem` 驱动的唯一前台状态机。账号、区服、角色列表、创建草稿、预览、进入世界和 UI 分层分别由专属 Subsystem/Controller 持有，Widget 不直接访问 HTTP、Token 或 ClientTravel。

本轮未删除 `.uasset`、`.umap` 或仍有引用的 Legacy 源码对象。旧类型与旧接口已增加 Deprecated 元数据或响应头；只有在 Editor Reference Viewer、Blueprint Compile、Fix Redirectors 和人工 E2E 均完成后才能继续物理删除。

## 关键修改

- `EDBAFrontendState` 与 `UDBAFrontendFlowSubsystem` 是唯一前台状态权威。
- `EDBAZodiac`、`EDBAElement`、`EDBAFiveCamp` 是 canonical 身份类型；旧 `*Type` 枚举只供迁移。
- FiveCamp 不推导 TeamId，也没有恢复 Faction/FactionSelect。
- `/api/v1/auth/login` 只做认证和 Token 签发，不再生成或修改玩家名。
- 新增 `/api/v1/auth/player-name/generate`；UE 注册后的首次认证编排显式调用，普通登录与 Refresh 不调用。
- `/api/v1/auth/player-name/ensure`、旧 Auth 路径和旧 Character 路径保留 Deprecated Adapter。
- 修正 UE5.8 `FCoreUObjectDelegates` include，并消除 Unity Build 匿名帮助函数重定义。
- 修正 GameEnter/Runtime 端点的命名空间编译阻塞。

## 工程验证

- UE5.8 Development Editor：编译成功。
- UE5.8 Development Server Win64：编译成功。
- .NET 10 `GameBackend.sln`：编译成功，0 警告、0 错误。
- `AuthServiceTests`：5/5 通过。
- `Game.ServerManagement.Tests`：5/5 通过。
- `Game.IntegrationTests`：2/2 通过。
- 全量 `Game.Api.Tests`：97/184 通过；其余失败为既有 InMemory 事务、WebApplicationFactory Host 和错误文本期望问题，不能记为全绿。

## 人工审核范围

环境启动后由人工完成以下操作，Agent 不自动输入、点击或判定业务通过：

1. 客户端冷启动并进入 Frontend。
2. 注册新账号，观察注册后只出现一次独立玩家名生成请求。
3. 确认玩家名为 3–5 个汉字。
4. 登出并使用普通账号登录，确认登录路径不再调用玩家名生成接口。
5. 进入 ServerSelect、角色列表和四步角色创建。
6. 检查 CharacterSelect/Create 预览、切换、返回和取消。
7. 选择角色并申请 EnterWorld；确认 Dedicated Server 接收连接。
8. 记录画面、API/DS/客户端日志和人工结论。

## 结论状态定义

- **环境已就绪**：API、Dedicated Server 与可见客户端已启动。
- **人工正在审核**：人工已开始输入、点击和观察。
- **人工确认通过**：人工明确反馈流程和画面通过。
- **人工发现失败**：记录失败步骤、画面与日志后停止后续操作并修复。

本报告生成时尚未取得人工结论，因此不得将“进程启动成功”表述为业务验收通过。
