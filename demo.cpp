#include "CoolDB.h"

int main() {
    FastKeyValueStore store;

    // Set some values
    store.set("name", "John Doe");
    store.set("email", "john@example.com");
    store.set("age", "30");

    /*
    std::cout << "Initial state:\n";
    store.printAllKeys();

    // Save to file
    store.save();  // Saves to "save.txt"

    // Delete a key
    store.remove("email");
    std::cout << "\nAfter deleting 'email':\n";
    store.printAllKeys();

    // Clear all data
    store.clear();
    std::cout << "\nAfter clearing:\n";
    store.printAllKeys();

    // Load from file
    store.load();  // Loads from "save.txt"
    std::cout << "\nAfter loading from file:\n";
    store.printAllKeys();
    */

    store.get("email");
    return 0;
}
