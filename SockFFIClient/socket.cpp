#include "socket.hpp"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

Socket::Socket(const char *socketFile) : socketFile(socketFile) {}
Socket::~Socket() {
    disconnect();
    unlink(socketFile);
}

bool Socket::connect() {
    struct sockaddr_un clientAddr = {0};
    socklen_t addrLen = sizeof(clientAddr);
    clientFd = -1;

    if ((clientFd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket error");
        return false;
    }

    clientAddr.sun_family = AF_UNIX;
    strncpy(clientAddr.sun_path, socketFile, sizeof(clientAddr.sun_path) - 1);
    std::cout << "Client: Connecting to " << socketFile << std::endl;
    if (::connect(clientFd, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) == -1) {
        perror("Connect error");
        return false;
    }

    std::cout << "Client: Connected to the server" << std::endl;
    return true;
}

void Socket::disconnect() {
    if (clientFd != -1) {
        close(clientFd);
        std::cout << "Client: Disconnected from the server" << std::endl;
    }
}

void Socket::sendData(const void *buf, size_t len) {
    int sendResult = send(clientFd, reinterpret_cast<const char *>(buf), len, 0);
    if (sendResult == -1)
        perror("send error");
}

void Socket::receiveData(void *buf, size_t len) {
    for (size_t totalReceived = 0; totalReceived != len;) {
        const auto received = recv(clientFd, reinterpret_cast<char *>(buf) + totalReceived, len - totalReceived, 0);
        if (received == -1)
            perror("recv error");
        totalReceived += received;
    }
}

template <> void Socket::sendValue<const char *, sizeof(uintptr_t)>(const char *str) { sendString(str); }
