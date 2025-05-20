//
// Created by Adil Gadi on 4/25/25.
//
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#define MAX_FDS 100
#define SERVER_PORT 8080
#define LOG(x) printf("%s\n", x)

int server_socket;

void close_server(int) {
    close(server_socket);
    LOG("Closed server successfully\n");
    exit(0);
}

int main(void) {
    signal(SIGINT, close_server);
    signal(SIGTERM, close_server);

    struct pollfd fds[MAX_FDS];
    memset(fds, 0, sizeof(fds));
    nfds_t len = 0;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    fds[0].fd = server_socket;
    fds[0].events = POLLIN;
    struct poll_fd* server_pollfd = fds;
    len++;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    const int success = bind(server_socket, (struct sockaddr *) &addr, sizeof addr);

    if (success < 0) {
        LOG("Unable to bind\n");
        close(server_socket);
        exit(1);
    }

    listen(server_socket, 10);

    printf("Listening on PORT: %d\n", SERVER_PORT);

    while (true) {
        // LOG("Waiting");
        int ready = poll(fds, len, 1000);

        if (ready < -1) {
            LOG("Error");
            continue;
        }

        if (ready == 0) {
            // LOG("Skipped");
            continue;
        }

        // LOG("Received");

        printf("len: %d\n", len);

        for (int i = 0; i < len; i++) {
            struct pollfd *socket_fd = fds + i;

            // printf("%p, %p\n", socket_fd, &fds[0]);
            // printf("%d\n", socket_fd->revents);

            if (socket_fd->revents & 0x1) {
                if (socket_fd == fds) {
                    // LOG("Server Socket");

                    const int client = accept(fds->fd, 0, 0);

                    if (client == -1) {
                        continue;
                    }

                    struct pollfd* client_fd = fds + len;
                    client_fd->fd = client;
                    client_fd->events = POLLIN;

                    len++;

                    LOG("New Client");

                    socket_fd->revents = 0;
                    continue;
                }

                char buffer[1024];
                const size_t req_size = recv(socket_fd->fd, &buffer, sizeof(buffer), 0);
                LOG("Received!");
                printf("Res Size: %lu\n", req_size);
                socket_fd->revents = 0;
                socket_fd->events = POLLOUT;
                continue;
            }

            if (socket_fd->revents == 0x4) {
                // LOG(socket_fd->revents);
                char response[128];
                snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nFrom server");
                response[sizeof(response) - 1] = 0;
                send(socket_fd->fd, &response, strlen(response), 0);
                LOG("Sent.");
                close(socket_fd->fd);
                memset(socket_fd, 0, sizeof(struct pollfd));
                socket_fd->fd = -1;
            }

            socket_fd->revents = 0;
        }
    }
}
