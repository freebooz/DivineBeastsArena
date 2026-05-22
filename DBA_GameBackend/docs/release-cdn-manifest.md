# CDN Manifest Release Flow

官网和启动器都读取 Game.Api 的 launcher manifest：

- 官网：`GET /api/launcher/manifest?channel=stable&platform=Windows`
- 启动器：`GET /launcher/manifest.json?channel=stable&platform=Windows`

运营发布补丁包时需要先把安装包上传到 CDN，然后通过 Admin API 写入版本元数据。

## 发布 Windows 启动器包

```bash
ADMIN_TOKEN="<admin jwt>"
BASE_URL="https://api.example.com"

curl -X POST "$BASE_URL/api/admin/client-versions" \
  -H "Authorization: Bearer $ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "version": "0.1.1",
    "channel": "stable",
    "platform": "Windows",
    "downloadUrl": "https://cdn.example.com/releases/DivineBeastsArena-Windows-0.1.1.zip",
    "checksum": "<sha256>",
    "sizeBytes": 123456789,
    "isMandatory": true,
    "isActive": true,
    "minOsVersion": "Windows 10",
    "releaseNotes": "修复启动器更新和登录流程。",
    "reason": "release 0.1.1"
  }'
```

规则：

- 同一个 `channel + platform` 只能有一个 active 版本。
- 发布 active 版本时，旧 active 版本会自动置为 inactive。
- 操作需要 Admin Token，并写入 `admin_audit_log`。
- CDN 文件上传和 SHA256 计算由发布流水线完成，Game.Api 只保存可信元数据。
