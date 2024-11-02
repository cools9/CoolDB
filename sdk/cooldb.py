import requests

class CoolDBClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url

    def set(self, key, value):
        """Sets a key-value pair in the database."""
        url = f"{self.base_url}/set"
        data = {"key": key, "value": value}
        response = requests.post(url, json=data)

        if response.status_code == 200:
            return response.json().get("message")
        else:
            raise Exception(f"Failed to set key-value pair: {response.json().get('error')}")

    def get(self, key):
        """Gets the value for a given key."""
        url = f"{self.base_url}/get/{key}"
        response = requests.get(url)

        if response.status_code == 200:
            return response.json().get("value")
        elif response.status_code == 404:
            return None
        else:
            raise Exception("Failed to retrieve value for key")

    def list_keys(self):
        """Lists all keys in the database."""
        url = f"{self.base_url}/list"
        response = requests.get(url)

        if response.status_code == 200:
            return response.json().get("keys", [])
        else:
            raise Exception("Failed to list keys")

    def status(self):
        """Checks the status of the database."""
        url = f"{self.base_url}/status"
        response = requests.get(url)

        if response.status_code == 200:
            return response.json().get("status")
        else:
            raise Exception("Failed to retrieve status")

# Example usage
if __name__ == "__main__":
    client = CoolDBClient()

    # Set key-value pair
    print(client.set("hello", "world"))

    # Get value by key
    print(client.get("hello"))

    # List all keys
    print(client.list_keys())

    # Check status
    print(client.status())
