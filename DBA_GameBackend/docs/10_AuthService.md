# 步骤 10：AuthService 认证服务

## 收敛结论

认证权威实现仍是 `Game.Api`、`Game.Application/Auth`、`Game.Infrastructure/Auth` 和 `Game.Infrastructure/Database/Auth` 的单一组合；本阶段没有新建平行 `AuthService` 项目。`account` 与 `refresh_token` 已由现有 `GameDbContext` 和 EF migrations 管理，字段 `password_hash`、`token_hash`、`expires_at`、`revoked_at` 已满足本阶段安全边界，因此不新增重复表或迁移。

| 既有实现 | 目标实现 | 迁移办法 | 状态 |
| --- | --- | --- | --- |
| `/api/auth/account/*` 与 `/api/auth/*` | `/api/v1/auth/*` | 新旧路由共用同一 Endpoint Handler 与 Application Use Case | ADAPT |
| `PasswordHasher` / `BcryptPasswordCredentialVerifier` | 密码凭据唯一实现 | 继续使用 BCrypt，不保存或记录明文密码 | KEEP |
| `JwtLoginCredentialIssuer` / `EfRefreshCredentialStore` | Access Token + Refresh Token 生命周期 | 继续使用 64 字节随机 Refresh Token，仅持久化 SHA-256 哈希；补充重放检测 | KEEP |
| `GameDbContext.Accounts` / `RefreshTokens` | 账号与刷新凭据唯一持久化 Owner | 复用现有实体、配置和 migrations | KEEP |

## v1 对外契约

| 方法 | 路径 | 认证 | 职责 |
| --- | --- | --- | --- |
| POST | `/api/v1/auth/register` | 否 | 注册账号并签发令牌对 |
| POST | `/api/v1/auth/login` | 否 | 使用账号标识和密码登录 |
| POST | `/api/v1/auth/refresh` | 否 | 原子轮换 Refresh Token |
| POST | `/api/v1/auth/logout` | Bearer | 撤销传入 Refresh Token；未传入时撤销本账号全部刷新凭据 |
| GET | `/api/v1/auth/me` | Bearer | 返回 DTO，不返回 EF Entity |

旧 `/api/auth` 路由保留兼容性；前台新业务应使用 v1 路径。注册与登录都受既有 `auth` 限流策略保护。

## 安全规则

- Access Token 默认有效期为 15 分钟，Refresh Token 有效期由 `Jwt:RefreshTokenExpiryDays` 配置。
- Refresh Token 由系统加密随机数生成，客户端只在安全存储中保存；服务端只保存哈希，日志不得记录 Token、Password 或 RefreshToken。
- 任一已撤销且未过期 Refresh Token 再次使用时，返回 `REFRESH_REUSED`，并撤销同账号其余活动 Refresh Token。
- 密码错误、账号不存在及空凭据均以 `INVALID_CREDENTIALS` 和相同文案返回，避免账号枚举。
- 禁用/封禁账号返回 `ACCOUNT_DISABLED`；过期或无效 Refresh Token 返回 `TOKEN_EXPIRED`。

## 测试边界

`Game.Api.Tests/AuthServiceTests` 覆盖服务层轮换、重放与凭据撤销；`Game.IntegrationTests/AuthRefreshTokenPersistenceTests` 覆盖 Refresh Token 在实际 DbContext 基础设施中的哈希持久化、轮换和重放撤销。HTTP WebApplicationFactory 的既有配置注入问题不在本步骤中扩大修复。
