#!/usr/bin/env python3

import json
import time
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer


HOST = "0.0.0.0"
PORT = 5059
ROOM_TTL_SECONDS = 3600


# In-memory room storage:
# {
#   "roomId": {
#       "host": {"ip": "...", "port": ...} or None,
#       "clients": [{"ip": "...", "port": ...}, ...],
#       "createdAt": <milliseconds>,
#       "_expiresAt": <seconds>
#   }
# }
rooms = {}
rooms_lock = threading.Lock()


def now_ms():
    return int(time.time() * 1000)


def now_s():
    return time.time()


def cleanup_expired_rooms():
    current = now_s()
    expired = []

    with rooms_lock:
        for room_id, room_data in rooms.items():
            if room_data.get("_expiresAt", 0) <= current:
                expired.append(room_id)

        for room_id in expired:
            del rooms[room_id]


def get_room(room_id):
    cleanup_expired_rooms()
    with rooms_lock:
        room = rooms.get(room_id)
        if room is None:
            return {"host": None, "clients": []}

        return {
            "host": room.get("host"),
            "clients": list(room.get("clients", [])),
            "createdAt": room.get("createdAt"),
        }


def put_room(room_id, room_data):
    room_copy = {
        "host": room_data.get("host"),
        "clients": list(room_data.get("clients", [])),
        "createdAt": room_data.get("createdAt", now_ms()),
        "_expiresAt": now_s() + ROOM_TTL_SECONDS,
    }

    with rooms_lock:
        rooms[room_id] = room_copy


class SignalingHandler(BaseHTTPRequestHandler):
    server_version = "SignalingServer/1.0"

    def _read_json(self):
        try:
            length = int(self.headers.get("Content-Length", "0"))
        except ValueError:
            length = 0

        raw = self.rfile.read(length) if length > 0 else b""
        if not raw:
            return None

        try:
            return json.loads(raw.decode("utf-8"))
        except (json.JSONDecodeError, UnicodeDecodeError):
            return None

    def _send_text(self, status, text):
        body = text.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _send_json(self, status, obj):
        body = json.dumps(obj, separators=(",", ":")).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        if self.path == "/register":
            self.handle_register()
            return

        if self.path == "/join":
            self.handle_join()
            return

        self._send_text(404, "Not Found")

    def do_GET(self):
        if self.path.startswith("/poll/"):
            self.handle_poll()
            return

        self._send_text(404, "Not Found")

    def handle_register(self):
        data = self._read_json()
        if not isinstance(data, dict):
            self._send_text(400, "Invalid JSON")
            return

        room_id = data.get("roomId")
        ip = data.get("ip")
        port = data.get("port")

        room_data = {
            "host": {"ip": ip, "port": port},
            "clients": [],
            "createdAt": now_ms(),
        }

        put_room(str(room_id), room_data)
        self._send_text(200, "Registered")

    def handle_join(self):
        cleanup_expired_rooms()

        data = self._read_json()
        if not isinstance(data, dict):
            self._send_text(400, "Invalid JSON")
            return

        room_id = str(data.get("roomId"))
        ip = data.get("ip")
        port = data.get("port")

        with rooms_lock:
            room_data = rooms.get(room_id)

            if room_data is None:
                self._send_text(404, "Room not found or not propagated yet")
                return

            exists = any(
                client.get("ip") == ip and client.get("port") == port
                for client in room_data.get("clients", [])
            )

            if not exists:
                room_data["clients"].append({"ip": ip, "port": port})

            # Refresh TTL on join, matching previous behavior
            room_data["_expiresAt"] = now_s() + ROOM_TTL_SECONDS

            host = room_data.get("host")

        self._send_json(200, host)

    def handle_poll(self):
        parts = self.path.split("/")
        room_id = parts[2] if len(parts) > 2 else ""

        room_data = get_room(room_id)
        self._send_json(200, room_data)

    def log_message(self, format_string, *args):
        print(f"{self.client_address[0]} - - [{self.log_date_time_string()}] {format_string % args}")


def main():
    cleanup_expired_rooms()
    server = ThreadingHTTPServer((HOST, PORT), SignalingHandler)
    print(f"Listening on http://{HOST}:{PORT}")
    server.serve_forever()


if __name__ == "__main__":
    main()

