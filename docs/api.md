# API Reference

Base URL: `http://localhost:8080`

## Authentication APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| POST | `/api/auth/guest-login` | Guest login with device ID | None |
| POST | `/api/auth/account/login` | Player account login | None |
| POST | `/api/auth/account/register` | Player account registration | None |
| POST | `/api/auth/dev-login` | Development login (disabled in production) | None |
| POST | `/api/auth/refresh` | Refresh access token | None |
| POST | `/api/auth/logout` | Logout and revoke refresh token | Bearer |
| GET | `/api/auth/me` | Get current account info | Bearer |
| POST | `/api/auth/external/steam` | Steam external auth (mock) | None |
| POST | `/api/auth/external/eos` | EOS external auth (mock) | None |

## Account Character APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| GET | `/api/account/characters` | List persisted account characters | Bearer |
| POST | `/api/account/characters` | Create a character in database | Bearer |
| POST | `/api/account/characters/{characterId}/select` | Select a character for next login | Bearer |

## Admin APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| POST | `/api/admin/auth/login` | Admin login and JWT issuance | None |
| GET | `/api/admin/players` | Player list | Bearer |
| GET | `/api/admin/players/{playerId}` | Player detail | Bearer |
| GET | `/api/admin/matches` | Match result list | Bearer |
| GET | `/api/admin/servers` | Game server instance list | Bearer |
| GET | `/api/admin/feedback` | Player feedback list | Bearer |
| GET | `/api/admin/support/tickets` | Support ticket list | Bearer |
| GET | `/api/admin/audit-logs` | Admin audit logs | Bearer |

## Launcher and Platform APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| GET | `/launcher/manifest.json` | Raw launcher manifest | None |
| GET | `/api/launcher/status` | Launcher service status | None |
| GET | `/api/platform/applications` | Platform application catalog | None |
| GET | `/api/operations/status` | Operations summary | None |
| POST | `/api/feedback/` | Submit player feedback | None |

### Request/Response Examples

**POST /api/auth/guest-login**
```json
// Request
{ "deviceId": "optional-device-uuid" }

// Response
{
  "data": {
    "accessToken": "jwt-token",
    "refreshToken": "base64-token",
    "playerId": "uuid",
    "nickname": "Player_name"
  },
  "success": true
}
```

**POST /api/auth/refresh**
```json
// Request
{ "refreshToken": "base64-token" }

// Response
{
  "data": {
    "accessToken": "new-jwt-token",
    "refreshToken": "new-base64-token"
  },
  "success": true
}
```

---

## Player APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| GET | `/api/players/me/profile` | Get current player profile | Bearer |
| GET | `/api/players/me/settings` | Get player settings | Bearer |
| GET | `/api/players/me/stats` | Get player statistics | Bearer |
| GET | `/api/players/me/unlocks` | Get player unlocks | Bearer |
| GET | `/api/players/{playerId}/public` | Get public player profile | None |

---

## Config APIs

### Client APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| GET | `/api/config/manifest` | Get config manifest | None |
| GET | `/api/config/{configKey}` | Get config by key | None |

### Admin APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| GET | `/api/admin/configs` | List all configs | Admin |
| POST | `/api/admin/configs` | Create new config | Admin |
| PUT | `/api/admin/configs/{id}` | Update config | Admin |
| POST | `/api/admin/configs/{id}/validate` | Validate config | Admin |
| POST | `/api/admin/configs/{id}/publish` | Publish config | Admin |
| POST | `/api/admin/configs/{id}/rollback` | Rollback config | Admin |

---

## Room APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| POST | `/api/rooms` | Create new room | Bearer |
| GET | `/api/rooms` | List rooms (filter by mode, region) | None |
| GET | `/api/rooms/{roomId}` | Get room details | None |
| POST | `/api/rooms/{roomId}/join` | Join room | Bearer |
| POST | `/api/rooms/{roomId}/leave` | Leave room | Bearer |
| POST | `/api/rooms/{roomId}/ready` | Set ready status | Bearer |
| POST | `/api/rooms/{roomId}/start` | Start game | Bearer |
| POST | `/api/rooms/{roomId}/kick` | Kick player | Bearer |
| POST | `/api/rooms/{roomId}/transfer-owner` | Transfer ownership | Bearer |

---

## Match APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| POST | `/api/matchmaking/tickets` | Create matchmaking ticket | Bearer |
| GET | `/api/matchmaking/tickets/{ticketId}` | Get ticket status | Bearer |
| DELETE | `/api/matchmaking/tickets/{ticketId}` | Cancel ticket | Bearer |
| POST | `/api/matchmaking/worker/tick` | Worker tick (internal) | Internal |

---

## Session APIs

### Client APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| GET | `/api/sessions/{sessionId}` | Get session details | Bearer |
| GET | `/api/sessions/{sessionId}/connection` | Get connection info | Bearer |
| POST | `/api/sessions/{sessionId}/reconnect-token` | Get reconnect token | Bearer |

### Internal APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| POST | `/internal/sessions/from-room` | Create session from room | Internal |
| POST | `/internal/sessions/from-match` | Create session from match | Internal |
| POST | `/internal/sessions/{sessionId}/allocate-server` | Allocate server | Internal |
| POST | `/internal/sessions/{sessionId}/mark-in-progress` | Mark in progress | Internal |
| POST | `/internal/sessions/{sessionId}/mark-completed` | Mark completed | Internal |
| POST | `/internal/sessions/{sessionId}/mark-failed` | Mark failed | Internal |

---

## Runtime APIs (Game Server)

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| POST | `/internal/runtime/servers` | Allocate server | Internal |
| GET | `/internal/runtime/servers/{serverId}` | Get server info | Internal |
| POST | `/internal/runtime/servers/{serverId}/heartbeat` | Server heartbeat | Internal |
| POST | `/internal/runtime/servers/{serverId}/ready` | Mark server ready | Internal |
| POST | `/internal/runtime/servers/{serverId}/stopped` | Mark server stopped | Internal |

---

## Settlement APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| POST | `/runtime/matches/results` | Submit match results | Runtime |
| GET | `/api/players/me/matches` | Get player matches | Bearer |
| GET | `/api/matches/{matchId}` | Get match details | None |

---

## Game Server APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| POST | `/internal/servers/register` | Register game server | Internal |
| GET | `/internal/servers/{serverId}` | Get server details | Internal |
| GET | `/internal/servers/active` | Get active servers | Internal |

---

## Inventory APIs

| Method | Endpoint | Description | Auth |
|--------|----------|-------------|------|
| GET | `/api/players/me/inventory` | Get player inventory | Bearer |
| GET | `/api/players/me/unlocks` | Get player unlocks | Bearer |
| POST | `/api/admin/inventory/grant` | Grant item (GM) | Admin |
| POST | `/api/admin/inventory/deduct` | Deduct item (GM) | Admin |
| GET | `/api/admin/inventory/logs` | Get inventory logs | Admin |

---

## Health & Monitoring

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/health/live` | Liveness probe |
| GET | `/health/ready` | Readiness probe |

---

## Error Response Format

All errors follow RFC 7807 Problem Details:

```json
{
  "type": "https://error.mygameplatform.com/auth/INVALID_CREDENTIALS",
  "title": "Invalid credentials",
  "detail": "The provided credentials are invalid",
  "status": 401
}
```

---

## Rate Limiting

- Public APIs: 100 requests/minute per IP
- Auth APIs: 10 requests/minute per IP
- Internal APIs: 1000 requests/minute per API key

---

## Authentication

### Bearer Token

```http
Authorization: Bearer <access-token>
```

### Internal API Key

```http
X-Internal-Api-Key: <api-key>
```
