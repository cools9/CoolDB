#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define INITIAL_CAPACITY 10
#define CAPACITY_INCREMENT 10

typedef struct {
    char **keys;
    char **values;
    int size;
    int capacity;
} SortedArray;

SortedArray *createSortedArray() {
    SortedArray *arr = (SortedArray *)malloc(sizeof(SortedArray));
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
    arr->keys = (char **)malloc(sizeof(char *) * arr->capacity);
    arr->values = (char **)malloc(sizeof(char *) * arr->capacity);
    return arr;
}

void ensureCapacity(SortedArray *arr) {
    if (arr->size >= arr->capacity) {
        arr->capacity += CAPACITY_INCREMENT;
        arr->keys = (char **)realloc(arr->keys, sizeof(char *) * arr->capacity);
        arr->values = (char **)realloc(arr->values, sizeof(char *) * arr->capacity);
    }
}

int binarySearch(SortedArray *arr, const char *key) {
    int left = 0, right = arr->size - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(arr->keys[mid], key);
        if (cmp == 0) return mid;
        if (cmp < 0) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

void insert(SortedArray *arr, const char *key, const char *value) {
    ensureCapacity(arr);
    int index = binarySearch(arr, key);
    if (index >= 0) {
        // Update existing key
        free(arr->values[index]);
        arr->values[index] = strdup(value);
        return;
    }

    // Find the position to insert
    index = -(index + 1); // Get the insert position
    for (int i = arr->size; i > index; i--) {
        arr->keys[i] = arr->keys[i - 1];
        arr->values[i] = arr->values[i - 1];
    }
    arr->keys[index] = strdup(key);
    arr->values[index] = strdup(value);
    arr->size++;
}

char *get(SortedArray *arr, const char *key) {
    int index = binarySearch(arr, key);
    if (index >= 0) {
        return arr->values[index];
    }
    return NULL;
}

void deleteKey(SortedArray *arr, const char *key) {
    int index = binarySearch(arr, key);
    if (index < 0) return; // Key not found

    free(arr->keys[index]);
    free(arr->values[index]);

    // Shift elements left
    for (int i = index; i < arr->size - 1; i++) {
        arr->keys[i] = arr->keys[i + 1];
        arr->values[i] = arr->values[i + 1];
    }
    arr->size--;
}

void freeSortedArray(SortedArray *arr) {
    for (int i = 0; i < arr->size; i++) {
        free(arr->keys[i]);
        free(arr->values[i]);
    }
    free(arr->keys);
    free(arr->values);
    free(arr);
}

double getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0); // Convert to milliseconds
}

void cli(SortedArray *arr) {
    char command[100];
    while (1) {
        printf("Enter command (SET key value, GET key, DEL key, EXIT): ");
        fgets(command, sizeof(command), stdin);

        if (strncmp(command, "EXIT", 4) == 0) {
            break;
        }

        char key[50], value[50];
        if (sscanf(command, "SET %s %s", key, value) == 2) {
            double start = getCurrentTime();
            insert(arr, key, value);
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
        } else {
            printf("Invalid command\n");
        }
    }
}

int main() {
    SortedArray *arr = createSortedArray();
    cli(arr);
    freeSortedArray(arr);
    return 0;
}
