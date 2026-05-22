# Production Secrets

DBA_GameBackend supports secret-file projection for centralized secret managers.

Any environment variable ending in `_FILE` is read at process startup. The suffix is removed and `__` is converted to `:` so ASP.NET Core receives the real configuration key.

Examples:

```bash
Jwt__Secret_FILE=/run/secrets/jwt_secret
InternalApi__Key_FILE=/run/secrets/internal_api_key
Database__ConnectionString_FILE=/run/secrets/database_connection_string
Redis__ConnectionString_FILE=/run/secrets/redis_connection_string
```

This works with Docker secrets, Kubernetes projected secrets, systemd credentials, Vault Agent templates, and similar tools. Keep the actual secret files outside git.
