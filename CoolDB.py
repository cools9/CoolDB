import socket
import time

class CoolDB:
    def __init__(self, ip, port):
        self.server_ip = ip
        self.server_port = port

    def _send_command(self, command):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.settimeout(5)  # 5 second timeout
                s.connect((self.server_ip, self.server_port))
                s.sendall(command.encode())
                response = s.recv(4096).decode()
                print(f"Raw response: {response}")  # Print raw response for debugging
                return response.strip()
        except Exception as e:
            print(f"Error in _send_command: {e}")
            return None

    def set(self, key, value, ttl=0):
        if ttl > 0:
            command = f"SET {key} {value} {ttl}"
        else:
            command = f"SET {key} {value}"
        return self._send_command(command)

    def get(self, key):
        command = f"GET {key}"
        return self._send_command(command)

def run_tests():
    db = CoolDB("127.0.0.1", 8000)

    print("Testing SET operation:")
    st = time.time()
    response = db.set("hello", "moto")
    et = time.time() - st
    print("SET response:", response)
    print("SET operation took:", et)

    print("\nTesting GET operation:")
    sg = time.time()
    response = db.get("hello")
    eg = time.time() - sg
    print("GET response:", response)
    print("GET operation took:", eg)

    print("\nTesting SET with TTL:")
    response = db.set("temp", "value", 10)  # Set with 10 second TTL
    print("SET with TTL response:", response)

    print("\nTesting GET after SET:")
    response = db.get("temp")
    print("GET after SET response:", response)

if __name__ == "__main__":
    run_tests()
