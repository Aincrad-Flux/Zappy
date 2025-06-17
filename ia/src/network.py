import socket

class NetworkClient:
    def __init__(self, host: str, port: int):
        self.host = host
        self.port = port
        self.sock = None
        self.buffer = ""

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))
        self.sock.setblocking(False)

    def send_command(self, command: str):
        print(f">>> {command.strip()}")
        self.sock.sendall((command + '\n').encode())

    def receive_response(self):
        try:
            data = self.sock.recv(4096).decode()
            self.buffer += data
            lines = self.buffer.split('\n')
            self.buffer = lines[-1]
            return lines[:-1]
        except BlockingIOError:
            return []
