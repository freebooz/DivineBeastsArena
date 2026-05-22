# 生产可用检查清单

此清单用于把平台从开发环境推进到可运营环境。每次发布前至少执行 `scripts/check-platform.ps1`，生产或预发环境再追加 `-LiveApi`。

## 后端与数据

- `DBA_GameBackend` 可以完成 `dotnet build` 和 `dotnet test`。
- 数据库迁移可以在目标环境成功执行。
- JWT 密钥、数据库连接串、Redis 连接串和内部服务密钥均来自环境变量或安全配置。
- 开发账号只存在于开发环境种子数据中，生产环境不得启用默认测试密码。
- `/health/live`、`/health/ready`、`/metrics` 可以被运维系统访问。

## 管理后台

- `DBA_GameAdmin` 能够连接目标 `Game.Api`。
- 写操作必须要求管理员身份和显式确认。
- GM 操作需要审计日志。
- 平台应用清单页面能读取 `/api/platform/applications`。

## 官网与启动器

- `DBA_GameWebsite` 可以完成 `npm run build`。
- 官网下载入口来自真实版本发布数据。
- `DBA_GameLauncher` 可以完成 `npm run build`。
- 启动器默认客户端路径指向 `DBA_GameClient` 或正式安装目录。
- 启动器清单接口返回版本、文件列表、校验值和下载地址。

## Unreal 客户端

- `DBA_GameClient/DivineBeastsArena.uproject` 可以打开并编译编辑器目标。
- `DBA_GameClient/Content/Splash/Splash.bmp` 和 `EdSplash.bmp` 存在并进入打包。
- 登录、游客登录、创建角色和选择角色均通过真实 API。
- 角色数据可写入数据库并在下次登录读取。
- 发布前使用 `scripts/check-platform.ps1 -IncludeGameClient` 验证编辑器目标编译。

## 运维

- `ops/docker/docker-compose.yml` 可以启动本地依赖和观测栈。
- Nginx、Prometheus、Grafana、Loki 配置与目标环境域名匹配。
- 备份和恢复脚本在预发库演练通过。
- 发布流程产生可追踪版本号、校验值和变更记录。
