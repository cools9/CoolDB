#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS 10
#define PORT 8000
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 1000

typedef struct {
    char *key;
    char *value;
    time_t expiry;
} KeyValue;

typedef struct {
    KeyValue **data;
    size_t size;
    size_t capacity;
} HashMap;

HashMap* createHashMap(size_t capacity) {
    HashMap* map = malloc(sizeof(HashMap));
    map->data = calloc(capacity, sizeof(KeyValue*));
    map->size = 0;
    map->capacity = capacity;
    return map;
}

size_t hash(const char *key) {
    size_t hash = 5381;
    int c;
    while ((c = *key++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

void insert(HashMap* map, const char *key, const char *value, int ttl) {
    size_t index = hash(key) % map->capacity;

    while (map->data[index] != NULL && strcmp(map->data[index]->key, key) != 0) {
        index = (index + 1) % map->capacity;
    }

    if (map->data[index] == NULL) {
        map->data[index] = malloc(sizeof(KeyValue));
        map->data[index]->key = strdup(key);
        map->size++;
    } else {
        free(map->data[index]->value);
    }

    map->data[index]->value = strdup(value);
    map->data[index]->expiry = ttl > 0 ? time(NULL) + ttl : 0;
}

char* get(HashMap* map, const char *key) {
    size_t index = hash(key) % map->capacity;

    while (map->data[index] != NULL) {
        if (strcmp(map->data[index]->key, key) == 0) {
            if (map->data[index]->expiry == 0 || map->data[index]->expiry > time(NULL)) {
                return map->data[index]->value;
            }
            return NULL;
        }
        index = (index + 1) % map->capacity;
    }

    return NULL;
}

void handleCommand(HashMap* db, char *command, int client_socket) {
    char response[BUFFER_SIZE] = {0};
    char key[50], value[50];
    int ttl;

    if (strncmp(command, "SET", 3) == 0) {
        if (sscanf(command, "SET %s %s %d", key, value, &ttl) == 3) {
            insert(db, key, value, ttl);
        } else if (sscanf(command, "SET %s %s", key, value) == 2) {
            insert(db, key, value, 0);
        }
        snprintf(response, sizeof(response), "SET %s %s\n", key, value);
    } else if (strncmp(command, "GET", 3) == 0) {
        if (sscanf(command, "GET %s", key) == 1) {
            char *result = get(db, key);
            if (result) {
                snprintf(response, sizeof(response), "GET %s = %s\n", key, result);
            } else {
                snprintf(response, sizeof(response), "GET %s = (not found)\n", key);
            }
        }
    } else {
        snprintf(response, sizeof(response), "Invalid command!\n");
    }

    send(client_socket, response, strlen(response), 0);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    HashMap* db = createHashMap(1000000);  // Larger initial capacity

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl: server_fd");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                if (client_socket == -1) {
                    perror("accept");
                    continue;
                }
                fcntl(client_socket, F_SETFL, O_NONBLOCK);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &ev) == -1) {
                    perror("epoll_ctl: client_socket");
                    exit(EXIT_FAILURE);
                }
            } else {
                char buffer[BUFFER_SIZE] = {0};
                ssize_t count = read(events[n].data.fd, buffer, sizeof(buffer));
                if (count == -1) {
                    if (errno != EAGAIN) {
                        perror("read");
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[n].data.fd, NULL);
                        close(events[n].data.fd);
                    }
                    continue;
                } else if (count == 0) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[n].data.fd, NULL);
                    close(events[n].data.fd);
                } else {
                    handleCommand(db, buffer, events[n].data.fd);
                }
            }
        }
    }

    return 0;
}
