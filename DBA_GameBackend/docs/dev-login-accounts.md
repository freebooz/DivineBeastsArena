# 开发测试账号

这些账号由 `DevelopmentDataSeeder` 在 `SeedData:Enabled=true` 时自动写入数据库。登录、角色创建、角色选择都必须连接正在运行的 `Game.Api`，不能绕过 API 直接进入大厅。

## 管理后台

| 用途 | 用户名 | 密码 | 入口 |
| --- | --- | --- | --- |
| 超级管理员 | `admin` | `Admin@123456` | `POST /api/admin/auth/login` |
| 运营管理员 | `ops_admin` | `Ops@123456` | `POST /api/admin/auth/login` |

## 游戏客户端开发账号

| 用户名 | 密码 | 默认昵称 |
| --- | --- | --- |
| `dba_dev_01` | `Dev@123456` | `DevTiger` |
| `dba_dev_02` | `Dev@123456` | `DevDragon` |
| `dba_dev_03` | `Dev@123456` | `DevRat` |

## 普通测试玩家

| 用户名范围 | 密码 | 说明 |
| --- | --- | --- |
| `player_001` 至 `player_020` | `Player@123456` | 常规玩家账号，默认可登录。 |
| `test_player_001` 至 `test_player_005` | `Test@123456` | 测试玩家账号，用于运营后台、背包、统计和工单验证。 |
| `test_player_003_banned` | `Test@123456` | 封禁状态账号，用于登录失败和后台状态验证。 |

## 外部登录模拟

| 类型 | 标识 | 密码/票据 |
| --- | --- | --- |
| Steam | `STEAM_MOCK_001` | `Steam@123456` |
| EOS | `EOS_MOCK_001` | `Eos@123456` |

## 快速验证

```powershell
$body = @{ username = "dba_dev_01"; password = "Dev@123456" } | ConvertTo-Json
Invoke-RestMethod -Method Post -Uri "http://localhost:8080/api/auth/account/login" -Body $body -ContentType "application/json"

$adminBody = @{ username = "admin"; password = "Admin@123456" } | ConvertTo-Json
Invoke-RestMethod -Method Post -Uri "http://localhost:8080/api/admin/auth/login" -Body $adminBody -ContentType "application/json"
```
