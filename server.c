#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define DETAILS_COUNT 3

char systemDetails[DETAILS_COUNT][1000];

void getSystemDetails() {
    // Get RAM details
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    snprintf(systemDetails[0], sizeof(systemDetails[0]),
             "RAM details: %llu MB available, %llu MB total",
             memInfo.ullAvailPhys / (1024 * 1024), memInfo.ullTotalPhys / (1024 * 1024));

    // Get Disk details
    ULARGE_INTEGER freeSpace, totalSpace, totalFreeSpace;
    GetDiskFreeSpaceEx("C:", &freeSpace, &totalSpace, &totalFreeSpace);
    snprintf(systemDetails[1], sizeof(systemDetails[1]),
             "Disk details: %llu GB free of %llu GB total",
             totalFreeSpace.QuadPart / (1024 * 1024 * 1024), totalSpace.QuadPart / (1024 * 1024 * 1024));

    // Get CPU details
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    snprintf(systemDetails[2], sizeof(systemDetails[2]),
             "CPU details: %d cores, %d threads",
             sysInfo.dwNumberOfProcessors, sysInfo.dwNumberOfProcessors * 2);
}

void handleClientCommand(const char *command, SOCKET client_socket) {
    char reply[1000] = {0};

    if (strcmp(command, "ram") == 0) {
        snprintf(reply, sizeof(reply), "%s", systemDetails[0]);
    } else if (strcmp(command, "disk") == 0) {
        snprintf(reply, sizeof(reply), "%s", systemDetails[1]);
    } else if (strcmp(command, "cpu") == 0) {
        snprintf(reply, sizeof(reply), "%s", systemDetails[2]);
    } else {
        snprintf(reply, sizeof(reply), "Server says: %s", command);
    }

    send(client_socket, reply, strlen(reply), 0);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;
    char buffer[1000] = {0};
    int sendInterval = 10; // Sending interval in seconds

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code: %d\n", WSAGetLastError());
        return 1;
    }

    listen(server_socket, 3);

    printf("Waiting for incoming connections...\n");

    c = sizeof(struct sockaddr_in);
    client_socket = accept(server_socket, (struct sockaddr*)&client, &c);
    if (client_socket == INVALID_SOCKET) {
        printf("Accept failed with error code: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Connection accepted\n");

    getSystemDetails();

    while (1) {
        // Check if there's any message from the client
        memset(buffer, 0, sizeof(buffer));
        int recv_size = recv(client_socket, buffer, sizeof(buffer), 0);
        if (recv_size == SOCKET_ERROR || recv_size == 0) {
            printf("Receive failed or connection closed\n");
            break;
        }

        printf("Client: %s\n", buffer);

        if (strcmp(buffer, "hii") == 0) {
            printf("Server input: ");
            fgets(buffer, sizeof(buffer), stdin);
            if (buffer[strlen(buffer) - 1] == '\n') {
                buffer[strlen(buffer) - 1] = '\0';
            }

            send(client_socket, buffer, strlen(buffer), 0);
        } else {
            handleClientCommand(buffer, client_socket);
        }

        // Sleep for 1 second
        Sleep(1000);
    }

    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
