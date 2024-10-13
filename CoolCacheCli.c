#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define INITIAL_CAPACITY 10
#define CAPACITY_INCREMENT 10

typedef struct {
    char *key;
    char *value;
    time_t expiry;  // Expiry time (0 if no expiry)
} KeyValue;

typedef struct {
    KeyValue **pairs;
    int size;
    int capacity;
} SortedArray;

SortedArray *createSortedArray() {
    SortedArray *arr = (SortedArray *)malloc(sizeof(SortedArray));
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
    arr->pairs = (KeyValue **)malloc(sizeof(KeyValue *) * arr->capacity);
    return arr;
}

void ensureCapacity(SortedArray *arr) {
    if (arr->size >= arr->capacity) {
        arr->capacity += CAPACITY_INCREMENT;
        arr->pairs = (KeyValue **)realloc(arr->pairs, sizeof(KeyValue *) * arr->capacity);
    }
}

int binarySearch(SortedArray *arr, const char *key) {
    int left = 0, right = arr->size - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(arr->pairs[mid]->key, key);
        if (cmp == 0) return mid;
        if (cmp < 0) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

void cleanupExpired(SortedArray *arr) {
    time_t now = time(NULL);
    for (int i = 0; i < arr->size; ) {
        if (arr->pairs[i]->expiry > 0 && arr->pairs[i]->expiry <= now) {
            free(arr->pairs[i]->key);
            free(arr->pairs[i]->value);
            free(arr->pairs[i]);
            for (int j = i; j < arr->size - 1; j++) {
                arr->pairs[j] = arr->pairs[j + 1];
            }
            arr->size--;
        } else {
            i++;
        }
    }
}

void insert(SortedArray *arr, const char *key, const char *value, int ttl) {
    ensureCapacity(arr);
    cleanupExpired(arr);

    int index = binarySearch(arr, key);
    if (index >= 0) {
        free(arr->pairs[index]->value);
        arr->pairs[index]->value = strdup(value);
        arr->pairs[index]->expiry = ttl > 0 ? time(NULL) + ttl : 0;
        return;
    }

    KeyValue *newPair = (KeyValue *)malloc(sizeof(KeyValue));
    newPair->key = strdup(key);
    newPair->value = strdup(value);
    newPair->expiry = ttl > 0 ? time(NULL) + ttl : 0;

    index = -(index + 1);
    for (int i = arr->size; i > index; i--) {
        arr->pairs[i] = arr->pairs[i - 1];
    }
    arr->pairs[index] = newPair;
    arr->size++;
}

char *get(SortedArray *arr, const char *key) {
    cleanupExpired(arr);
    int index = binarySearch(arr, key);
    if (index >= 0 && (arr->pairs[index]->expiry == 0 || arr->pairs[index]->expiry > time(NULL))) {
        return arr->pairs[index]->value;
    }
    return NULL;
}

void deleteKey(SortedArray *arr, const char *key) {
    cleanupExpired(arr);
    int index = binarySearch(arr, key);
    if (index < 0) return;

    free(arr->pairs[index]->key);
    free(arr->pairs[index]->value);
    free(arr->pairs[index]);

    for (int i = index; i < arr->size - 1; i++) {
        arr->pairs[i] = arr->pairs[i + 1];
    }
    arr->size--;
}

void clear(SortedArray *arr) {
    for (int i = 0; i < arr->size; i++) {
        free(arr->pairs[i]->key);
        free(arr->pairs[i]->value);
        free(arr->pairs[i]);
    }
    arr->size = 0;
}

void mset(SortedArray *arr, int count, char **keysAndValues) {
    for (int i = 0; i < count; i += 2) {
        insert(arr, keysAndValues[i], keysAndValues[i + 1], 0);
    }
}

void mget(SortedArray *arr, int count, char **keys) {
    for (int i = 0; i < count; i++) {
        char *value = get(arr, keys[i]);
        if (value) {
            printf("%s = %s\n", keys[i], value);
        } else {
            printf("%s = (not found)\n", keys[i]);
        }
    }
}

void keys(SortedArray *arr) {
    cleanupExpired(arr);
    for (int i = 0; i < arr->size; i++) {
        printf("%s\n", arr->pairs[i]->key);
    }
}

int size(SortedArray *arr) {
    cleanupExpired(arr);
    return arr->size;
}

double getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0); // Convert to milliseconds
}

void cli(SortedArray *arr) {
    char command[256];
    while (1) {
        printf("Enter command: ");
        fgets(command, sizeof(command), stdin);
        if (strncmp(command, "EXIT", 4) == 0) break;

        char key[50], value[50];
        int ttl;

        if (sscanf(command, "SET %s %s %d", key, value, &ttl) == 3) {
            double start = getCurrentTime();
            insert(arr, key, value, ttl);
            double end = getCurrentTime();
            printf("SET %s %s with TTL %d took %.2f ms\n", key, value, ttl, end - start);
        } else if (sscanf(command, "SET %s %s", key, value) == 2) {
            double start = getCurrentTime();
            insert(arr, key, value, 0);
            double end = getCurrentTime();
            printf("SET %s %s took %.2f ms\n", key, value, end - start);
        } else if (sscanf(command, "GET %s", key) == 1) {
            double start = getCurrentTime();
            char *result = get(arr, key);
            double end = getCurrentTime();
            if (result) {
                printf("GET %s = %s, took %.2f ms\n", key, result, end - start);
            } else {
                printf("GET %s = (not found), took %.2f ms\n", key, end - start);
            }
        } else if (sscanf(command, "DEL %s", key) == 1) {
            double start = getCurrentTime();
            deleteKey(arr, key);
            double end = getCurrentTime();
            printf("DEL %s took %.2f ms\n", key, end - start);
        } else if (strncmp(command, "KEYS", 4) == 0) {
            double start = getCurrentTime();
            keys(arr);
            double end = getCurrentTime();
            printf("KEYS took %.2f ms\n", end - start);
        } else if (strncmp(command, "CLEAR", 5) == 0) {
            double start = getCurrentTime();
            clear(arr);
            double end = getCurrentTime();
            printf("CLEAR took %.2f ms\n", end - start);
        } else if (strncmp(command, "SIZE", 4) == 0) {
            double start = getCurrentTime();
            int numKeys = size(arr);
            double end = getCurrentTime();
            printf("SIZE = %d, took %.2f ms\n", numKeys, end - start);
        } else if (strncmp(command, "CLEANUP", 7) == 0) {
            double start = getCurrentTime();
            cleanupExpired(arr);
            double end = getCurrentTime();
            printf("CLEANUP took %.2f ms\n", end - start);
        } else if (strncmp(command, "MSET", 4) == 0) {
            char *token = strtok(command + 5, " ");
            char *args[20];
            int count = 0;
            while (token != NULL) {
                args[count++] = token;
                token = strtok(NULL, " ");
            }
            double start = getCurrentTime();
            mset(arr, count, args);
            double end = getCurrentTime();
            printf("MSET took %.2f ms\n", end - start);
        } else if (strncmp(command, "MGET", 4) == 0) {
            char *token = strtok(command + 5, " ");
            char *args[10];
            int count = 0;
            while (token != NULL) {
                args[count++] = token;
                token = strtok(NULL, " ");
            }
            double start = getCurrentTime();
            mget(arr, count, args);
            double end = getCurrentTime();
            printf("MGET took %.2f ms\n", end - start);
        } else {
            printf("Invalid command!\n");
        }
    }
}

int main() {
    SortedArray *db = createSortedArray();
    cli(db);
    clear(db);  // Cleanup memory
    free(db->pairs);
    free(db);
    return 0;
}
