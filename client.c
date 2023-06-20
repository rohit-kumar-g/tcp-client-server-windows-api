#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server;
    char message[1000] = {0};
    char server_reply[1000] = {0};

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    if (connect(client_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        puts("Connection error");
        return 1;
    }

    printf("Connected to the server\n");

    while (1) {
        printf("Enter a command (ram/disk/cpu): ");
        fgets(message, sizeof(message), stdin);
        if (message[strlen(message) - 1] == '\n') {
            message[strlen(message) - 1] = '\0'; // Remove newline character
        }

        send(client_socket, message, strlen(message), 0);

        memset(server_reply, 0, sizeof(server_reply));
        int recv_size = recv(client_socket, server_reply, sizeof(server_reply), 0);
        if (recv_size == SOCKET_ERROR || recv_size == 0) {
            printf("Receive failed or connection closed\n");
            break;
        }

        printf("Server: %s\n", server_reply);
    }

    closesocket(client_socket);
    WSACleanup();

    return 0;
}
