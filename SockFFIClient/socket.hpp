#pragma once

#include <string>
#include <fstream>
#include <cstdint>
#include <cstddef>

class Socket {
    int clientFd = -1;
    const char *socketFile;

    std::ofstream trafficDump;

public:
    Socket(const char *socketFile);
    ~Socket();

    bool connect();
    void disconnect();

    void sendData(const void *buf, size_t len);
    void sendString(std::string_view str) {
        sendValue<uint16_t, 2>(str.size());
        sendData(str.data(), str.size());
    }
    template <typename T, unsigned size> void sendValue(T v) {
        static_assert(sizeof(T) == size);
        sendData(&v, size);
    }

    void receiveData(void *buf, size_t len);
    template <typename T, unsigned size> T receiveValue() {
        static_assert(sizeof(T) == size);
        T v;
        receiveData(&v, size);
        return v;
    }
    std::string receiveString() {
        std::string fres(static_cast<size_t>(receiveValue<uint16_t, 2>()), '\0');
        receiveData(fres.data(), fres.size());
        return fres;
    }
};

template <> void Socket::sendValue<const char *, sizeof(uintptr_t)>(const char *);
