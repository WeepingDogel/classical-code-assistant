"""
Simple AI gateway using Python's built-in http.server module.

The Windows 2000 client sends POST /v1/chat with a JSON body
{"message":"..."} and expects a JSON response
{"response":"..."}.

This echo stub replies with the user's own message so that
the chat loop can be tested without a real AI provider.
"""

import json
from http.server import HTTPServer, BaseHTTPRequestHandler

HOST = "0.0.0.0"
PORT = 8000


class GatewayHandler(BaseHTTPRequestHandler):
    """Handles GET / and POST /v1/chat requests."""

    def _send_json(self, status, obj):
        body = json.dumps(obj).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        """Return a simple status for connection tests."""
        self._send_json(200, {"status": "ok"})

    def do_POST(self):
        """
        Accept a chat request, echo the user message back
        as the 'response' field.
        """
        try:
            length = int(self.headers.get("Content-Length", 0))
            raw = self.rfile.read(length).decode("utf-8")
            data = json.loads(raw)
            message = data.get("message", "")
        except Exception:
            self._send_json(400, {"error": "Invalid JSON body."})
            return

        # Echo stub: replace with a real AI provider call.
        reply = "You said: " + message
        self._send_json(200, {"response": reply})

    def log_message(self, fmt, *args):
        """Suppress per-request logging to keep output clean."""
        pass


def main():
    server = HTTPServer((HOST, PORT), GatewayHandler)
    print("Gateway listening on http://%s:%d" % (HOST, PORT))
    server.serve_forever()


if __name__ == "__main__":
    main()