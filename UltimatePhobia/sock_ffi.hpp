#pragma once

#include "ffi.hpp"

#include <winsock2.h>
#include <string>
#include <string_view>
#include <filesystem>

class SockFFI final : public FFI {
    std::string path;
    SOCKET clientSocket, listenSocket;

    void sendData(const void *, size_t);
    void sendString(std::string_view);
    template <typename T> void sendValue(T v) { sendData(&v, sizeof(v)); }

    void receiveData(void *, size_t);
    std::string receiveString();
    template <typename T> void receiveValue(T& v) { receiveData(&v, sizeof(v)); }
    template <typename T> T receiveValue() {
        T v;
        receiveData(&v, sizeof(v));
    }

public:
    SockFFI(const std::filesystem::path& modPath);
    ~SockFFI();

    void simpleCall(const char *name);
};
