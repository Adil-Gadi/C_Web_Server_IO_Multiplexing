#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unistd.h"
#include "sys/socket.h"
#include "sys/select.h"
#include "netinet/in.h"

#define SERVER_PORT 8080
#define MAX_CLIENTS 20
#define LOG(x) printf("%s\n", x)

int server_socket;

void close_server(int) {
    close(server_socket);
    LOG("Closed server successfully\n");
    exit(0);
}

int main(void) {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    fd_set read_fds;
    fd_set write_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    FD_SET(server_socket, &read_fds);

    signal(SIGINT, close_server);
    signal(SIGTERM, close_server);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    int success = bind(server_socket, (struct sockaddr *) &addr, sizeof addr);

    if (success < 0) {
        LOG("Unable to bind\n");
        close(server_socket);
        exit(1);
    }

    listen(server_socket, 10);

    printf("Listening on PORT: %d\n", SERVER_PORT);

    while (true) {
        fd_set read_fds_temp = read_fds;
        fd_set write_fds_temp = write_fds;

        LOG("waiting...");
        const int result = select(MAX_CLIENTS + 1, &read_fds_temp, &write_fds_temp, NULL, NULL);

        if (result < 0) {
            perror("Error using select");
            close(server_socket);
            exit(1);
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            int isReadSocket = FD_ISSET(i, &read_fds);
            int isWriteSocket = FD_ISSET(i, &write_fds);

            if (!isReadSocket && !isWriteSocket) {
                // LOG("Neither");
                continue;
            }

            if (i == server_socket) {
                int client = accept(server_socket, 0, 0);

                if (client == -1) {
                    continue;
                }

                FD_SET(client, &read_fds);

                LOG("Received");

                continue;
            }

            if (isReadSocket) {
                char buffer[1024];
                LOG("Receiving...");
                const size_t req_size = recv(i, &buffer, sizeof(buffer), 0);
                // printf("Res Size: %lu\n", req_size);
                FD_SET(i, &write_fds);
                // printf("FD_ISSET, %d\n", FD_ISSET(i, &write_fds));
                LOG("Received.");
                FD_CLR(i, &read_fds);

                char response[128];
                snprintf(response, sizeof(response), "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nFrom server");
                response[sizeof(response) - 1] = 0;
                LOG("Sending...");
                send(i, &response, strlen(response), 0);
                LOG("Sent.");
                FD_CLR(i, &write_fds);
                close(i);
            }
        }
    }
}