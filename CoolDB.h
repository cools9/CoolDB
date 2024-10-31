#include <iostream>
#include <vector>
#include <chrono>
#include <string_view>
#include <bit>
#include <cstring>
#include <memory>
#include <array>
#include <fstream>
#include <filesystem>

class FastKeyValueStore {
private:
    static constexpr size_t BUCKET_COUNT = 16384;
    static constexpr size_t MAX_KEY_LENGTH = 64;
    static constexpr size_t MAX_VALUE_LENGTH = 256;
    static constexpr const char* DEFAULT_SAVE_FILE = "save.txt";

    struct Entry {
        char key[MAX_KEY_LENGTH] = {0};
        char value[MAX_VALUE_LENGTH] = {0};
        bool occupied = false;
        uint64_t hash = 0;
    };

    std::array<Entry, BUCKET_COUNT> buckets;

    // Existing hash and comparison functions remain the same
    static uint64_t computeHash(std::string_view str) {
        uint64_t hash = 0xcbf29ce484222325;
        for (char c : str) {
            hash ^= static_cast<uint64_t>(c);
            hash *= 0x100000001b3;
        }
        return hash;
    }

    static bool fastStringCompare(const char* a, const char* b, size_t len) {
        const uint64_t* a64 = reinterpret_cast<const uint64_t*>(a);
        const uint64_t* b64 = reinterpret_cast<const uint64_t*>(b);

        size_t chunks = len / 8;
        for (size_t i = 0; i < chunks; ++i) {
            if (a64[i] != b64[i]) return false;
        }

        for (size_t i = chunks * 8; i < len; ++i) {
            if (a[i] != b[i]) return false;
        }
        return true;
    }

    size_t findBucket(std::string_view key, uint64_t hash) const {
        size_t index = hash & (BUCKET_COUNT - 1);
        while (buckets[index].occupied) {
            if (buckets[index].hash == hash &&
                fastStringCompare(buckets[index].key, key.data(), key.length())) {
                return index;
            }
            index = (index + 1) & (BUCKET_COUNT - 1);
        }
        return index;
    }

public:
    // Existing set, get, and printAllKeys methods remain the same
    void set(std::string_view key, std::string_view value) {
        if (key.length() >= MAX_KEY_LENGTH || value.length() >= MAX_VALUE_LENGTH) {
            throw std::runtime_error("Key or value too long");
        }

        auto start = std::chrono::steady_clock::now();

        uint64_t hash = computeHash(key);
        size_t index = findBucket(key, hash);

        Entry& entry = buckets[index];
        entry.occupied = true;
        entry.hash = hash;
        std::memcpy(entry.key, key.data(), key.length());
        entry.key[key.length()] = '\0';  // Ensure null termination
        std::memcpy(entry.value, value.data(), value.length());
        entry.value[value.length()] = '\0';  // Ensure null termination

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "Time taken for set(nanoseconds): " << duration << '\n';
    }

    std::string_view get(std::string_view key) const {
        if (key.length() >= MAX_KEY_LENGTH) {
            throw std::runtime_error("Key too long");
        }

        auto start = std::chrono::steady_clock::now();

        uint64_t hash = computeHash(key);
        size_t index = findBucket(key, hash);

        if (!buckets[index].occupied) {
            throw std::runtime_error("Key not found");
        }

        auto result = std::string_view(buckets[index].value);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "Time taken for get(nanoseconds): " << duration << '\n';

        return result;
    }

    // New method to delete a key
    bool remove(std::string_view key) {
        if (key.length() >= MAX_KEY_LENGTH) {
            throw std::runtime_error("Key too long");
        }

        uint64_t hash = computeHash(key);
        size_t index = findBucket(key, hash);

        if (!buckets[index].occupied) {
            return false;  // Key not found
        }

        buckets[index].occupied = false;
        std::memset(buckets[index].key, 0, MAX_KEY_LENGTH);
        std::memset(buckets[index].value, 0, MAX_VALUE_LENGTH);
        return true;
    }

    // New method to clear all entries
    void clear() {
        for (auto& bucket : buckets) {
            bucket.occupied = false;
            std::memset(bucket.key, 0, MAX_KEY_LENGTH);
            std::memset(bucket.value, 0, MAX_VALUE_LENGTH);
        }
        std::cout << "Store cleared successfully\n";
    }

    // New method to save to file
    void save(const std::string& filename = DEFAULT_SAVE_FILE) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file for writing: " + filename);
        }

        for (const auto& bucket : buckets) {
            if (bucket.occupied) {
                file << bucket.key << '\t' << bucket.value << '\n';
            }
        }
        std::cout << "Store saved to " << filename << "\n";
    }

    // New method to load from file
    void load(const std::string& filename = DEFAULT_SAVE_FILE) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Cannot open file for reading: " + filename);
        }

        clear();  // Clear existing data before loading

        std::string line, key, value;
        while (std::getline(file, line)) {
            size_t tab_pos = line.find('\t');
            if (tab_pos != std::string::npos) {
                key = line.substr(0, tab_pos);
                value = line.substr(tab_pos + 1);
                set(key, value);
            }
        }
        std::cout << "Store loaded from " << filename << "\n";
    }

    void printAllKeys() const {
        std::cout << "All keys in store:\n";
        int keyCount = 0;

        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            if (buckets[i].occupied) {
                std::cout << "[" << keyCount++ << "] " << buckets[i].key << "\n";
            }
        }

        if (keyCount == 0) {
            std::cout << "Store is empty\n";
        } else {
            std::cout << "Total keys: " << keyCount << "\n";
        }
    }
};
