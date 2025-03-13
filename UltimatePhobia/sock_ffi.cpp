#include "sock_ffi.hpp"
#include "global_state.hpp"
#include "ffi_interface.hpp"

#include <string>
#include <stdexcept>
#include <cstring>
#include <windows.h>
#include <ws2tcpip.h>
#include <afunix.h>

SockFFI::SockFFI(const std::filesystem::path& modPath) : path(modPath.string()) {
    path.pop_back(); // Remove the 'i'

    clientSocket = INVALID_SOCKET;
    listenSocket = INVALID_SOCKET;
    int result = 0;
    SOCKADDR_UN serverSocket = {0};

    // Initialize Winsock
    static const auto wsaResult = []() {
        WSADATA wsaData = {0};
        return WSAStartup(MAKEWORD(2, 2), &wsaData);
    }();
    if (wsaResult != 0)
        throw std::runtime_error("[SockFFI] WSAStartup failed with error: " + std::to_string(result));

    // Create a AF_UNIX stream server socket.
    listenSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        throw std::runtime_error("[SockFFI] socket failed with error: " + std::to_string(WSAGetLastError()));

    serverSocket.sun_family = AF_UNIX;
    strncpy_s(serverSocket.sun_path, sizeof serverSocket.sun_path, path.c_str(), path.size());

    // Bind the socket to the path.
    result = bind(listenSocket, (struct sockaddr *)&serverSocket, sizeof(serverSocket));
    if (result == SOCKET_ERROR)
        throw std::runtime_error("[SockFFI] bind failed with error: " + std::to_string(WSAGetLastError()));

    // Listen to start accepting connections.
    result = listen(listenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR)
        throw std::runtime_error("[SockFFI] list failed with error: " + std::to_string(WSAGetLastError()));

    g.logger->info("[SockFFI] Accepting connections on: '{}'", path);
    // Accept a connection.
    clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET)
        throw std::runtime_error("[SockFFI] accept failed with error: " + std::to_string(WSAGetLastError()));
    g.logger->info("[SockFFI] Accepted a connection.");
}

SockFFI::~SockFFI() {
    g.logger->info("[SockFFI] Shutting down.");
    shutdown(clientSocket, 0);

    if (listenSocket != INVALID_SOCKET)
        closesocket(listenSocket);

    if (clientSocket != INVALID_SOCKET)
        closesocket(clientSocket);

    // Analogous to `unlink`
    DeleteFileA(path.c_str());
}

void SockFFI::sendData(const void *buf, size_t len) {
    int sendResult = send(clientSocket, reinterpret_cast<const char *>(buf), len, 0);
    if (sendResult == SOCKET_ERROR)
        throw std::runtime_error("[SockFFI] send failed with error: " + std::to_string(WSAGetLastError()));
}
void SockFFI::sendString(std::string_view str) {
    sendValue<uint16_t, 2>(str.size());
    sendData(str.data(), str.size());
}

void SockFFI::receiveData(void *buf, size_t len) {
    for (size_t totalReceived = 0; totalReceived != len;) {
        const auto received = recv(clientSocket, reinterpret_cast<char *>(buf) + totalReceived, len - totalReceived, 0);
        if (received == -1)
            throw std::runtime_error("[SockFFI] recv failed with error: " + std::to_string(WSAGetLastError()));
        totalReceived += received;
    }
}

std::string SockFFI::receiveString() {
    std::string fres(static_cast<size_t>(receiveValue<uint16_t, 2>()), '\0');
    receiveData(fres.data(), fres.size());
    return fres;
}

void SockFFI::simpleCall(const char *name) {
    sendValue<bool, 1>(false); // New function should be executed
    sendString(name);
    if (!receiveValue<bool, 1>()) {
        g.logger->error("[SockFFI] Failed to call function {} in {} as it wasn't available", name, path);
        return;
    }

    for (;;) {
        // Get called function index
        const auto fncIdx = receiveValue<uint8_t, 1>();

        // Stop if function indicates exit
        if (fncIdx == 0)
            break;

        // Call specified function
        uint8_t curFncIdx = 0;
        std::apply([this, &curFncIdx, fncIdx](auto... args) { ((doRpcCall(args, ++curFncIdx, fncIdx)), ...); }, FFIInterface::functions);
        if (curFncIdx != std::tuple_size<decltype(FFIInterface::functions)>::value + 1)
            g.logger->warn("[SockFFI] Not all FFI functions were considered for calling");
    }

    g.logger->flush();
}
