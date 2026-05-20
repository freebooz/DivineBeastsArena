#!/usr/bin/env python3
import json
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse


HOST = "127.0.0.1"
PORT = 5000
ROOM_ID = "local_room_001"
SESSION_ID = "local_session_001"
TICKET_ID = "local_ticket_001"


ROOM = {
    "roomId": ROOM_ID,
    "id": ROOM_ID,
    "mode": "default",
    "region": "local",
    "maxPlayers": 10,
    "private": False,
    "status": "open",
    "players": [
        {
            "playerId": "frontend_debug_player",
            "displayName": "frontend_debug",
            "ready": False,
            "owner": True,
        }
    ],
}


def make_envelope(data=None, code="OK", message="", success=True):
    return {
        "success": success,
        "code": code,
        "message": message,
        "data": data if data is not None else {},
    }


class Handler(BaseHTTPRequestHandler):
    server_version = "MockGameBackend/0.1"

    def _read_json(self):
        length = int(self.headers.get("Content-Length", "0"))
        if length <= 0:
            return {}
        raw = self.rfile.read(length)
        if not raw:
            return {}
        try:
            return json.loads(raw.decode("utf-8"))
        except Exception:
            return {}

    def _send(self, status, payload):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, fmt, *args):
        return

    def do_GET(self):
        path = urlparse(self.path).path

        if path == "/api/rooms":
            self._send(200, make_envelope({"rooms": [ROOM]}))
            return

        if path == f"/api/rooms/{ROOM_ID}":
            self._send(200, make_envelope(ROOM))
            return

        if path == "/api/players/me/profile":
            profile = {
                "playerId": "frontend_debug_player",
                "displayName": "frontend_debug",
                "level": 8,
                "experience": 1200,
            }
            self._send(200, make_envelope(profile))
            return

        if path == "/api/config/bundle":
            bundle = {
                "configVersion": "bootstrap_v1",
                "maintenance": {"enabled": False},
            }
            self._send(200, make_envelope(bundle))
            return

        if path == "/api/players/me/settings":
            self._send(200, make_envelope({"language": "zh-CN", "region": "local"}))
            return

        if path == "/api/players/me/stats":
            self._send(200, make_envelope({"matches": 0, "wins": 0}))
            return

        if path == "/api/players/me/unlocks":
            self._send(200, make_envelope({"zodiacs": ["Rat", "Ox"], "skins": []}))
            return

        if path == "/api/players/me/inventory":
            self._send(200, make_envelope({"gold": 1280, "tickets": 6, "items": []}))
            return

        if path == "/api/players/me/matches":
            self._send(200, make_envelope({"matches": []}))
            return

        if path == f"/api/matchmaking/tickets/{TICKET_ID}":
            self._send(
                200,
                make_envelope(
                    {
                        "ticketId": TICKET_ID,
                        "id": TICKET_ID,
                        "status": "matched",
                        "sessionId": SESSION_ID,
                    }
                ),
            )
            return

        if path == f"/api/sessions/{SESSION_ID}":
            self._send(200, make_envelope({"sessionId": SESSION_ID, "status": "ready"}))
            return

        if path == f"/api/sessions/{SESSION_ID}/connection":
            self._send(
                200,
                make_envelope(
                    {
                        "ip": "127.0.0.1",
                        "port": 7777,
                        "sessionId": SESSION_ID,
                        "playerSessionToken": "mock_player_session",
                    }
                ),
            )
            return

        if path == "/api/health":
            self._send(200, make_envelope({"status": "up"}))
            return

        # Crash scan may call auth/me when running tests.
        if path == "/api/auth/me":
            self._send(200, make_envelope({"playerId": "frontend_debug_player"}))
            return

        self._send(404, make_envelope(code="NOT_FOUND", message="Not Found", success=False))

    def do_POST(self):
        path = urlparse(self.path).path
        payload = self._read_json()

        if path == "/api/auth/dev-login":
            display_name = payload.get("displayName") or payload.get("display_name") or "frontend_debug"
            data = {
                "accessToken": f"mock_access_{display_name}",
                "refreshToken": f"mock_refresh_{display_name}",
                "playerId": f"{display_name}_player",
            }
            self._send(200, make_envelope(data))
            return

        if path == "/api/auth/refresh":
            self._send(
                200,
                make_envelope(
                    {
                        "accessToken": "mock_access_refreshed",
                        "refreshToken": "mock_refresh_refreshed",
                        "playerId": "frontend_debug_player",
                    }
                ),
            )
            return

        if path == "/api/rooms":
            room = dict(ROOM)
            if payload.get("mode"):
                room["mode"] = payload.get("mode")
            if payload.get("region"):
                room["region"] = payload.get("region")
            if payload.get("maxPlayers"):
                room["maxPlayers"] = payload.get("maxPlayers")
            room["private"] = bool(payload.get("private", False))
            self._send(200, make_envelope(room))
            return

        if path.startswith(f"/api/rooms/{ROOM_ID}/"):
            action = path.rsplit("/", 1)[-1]
            room = dict(ROOM)
            if action == "ready":
                room["players"] = [dict(ROOM["players"][0], ready=bool(payload.get("ready", False)))]
            if action == "start":
                room["status"] = "started"
                room["sessionId"] = SESSION_ID
            self._send(200, make_envelope(room))
            return

        if path == "/api/matchmaking/tickets":
            self._send(
                200,
                make_envelope(
                    {
                        "ticketId": TICKET_ID,
                        "id": TICKET_ID,
                        "mode": payload.get("mode", "default"),
                        "region": payload.get("region", "local"),
                        "status": "matched",
                        "sessionId": SESSION_ID,
                    }
                ),
            )
            return

        if path == f"/api/sessions/{SESSION_ID}/reconnect-token":
            self._send(200, make_envelope({"reconnectToken": "mock_reconnect"}))
            return

        if path in (
            "/api/auth/logout",
            "/api/telemetry/batch",
            "/api/crashes/upload",
            "/api/client-logs/upload",
        ):
            self._send(200, make_envelope({}))
            return

        self._send(404, make_envelope(code="NOT_FOUND", message="Not Found", success=False))

    def do_PUT(self):
        path = urlparse(self.path).path
        if path == "/api/players/me/settings":
            self._send(200, make_envelope(self._read_json()))
            return
        self._send(200, make_envelope({}))

    def do_PATCH(self):
        path = urlparse(self.path).path
        if path == "/api/players/me/profile":
            profile = {
                "playerId": "frontend_debug_player",
                "displayName": self._read_json().get("displayName", "frontend_debug"),
                "level": 8,
                "experience": 1200,
            }
            self._send(200, make_envelope(profile))
            return
        self._send(200, make_envelope({}))

    def do_DELETE(self):
        path = urlparse(self.path).path
        if path == f"/api/matchmaking/tickets/{TICKET_ID}":
            self._send(200, make_envelope({"ticketId": TICKET_ID, "status": "cancelled"}))
            return
        self._send(200, make_envelope({}))


def main():
    httpd = ThreadingHTTPServer((HOST, PORT), Handler)
    print(f"Mock backend listening on http://{HOST}:{PORT}", flush=True)
    try:
        httpd.serve_forever()
    finally:
        httpd.server_close()


if __name__ == "__main__":
    main()
