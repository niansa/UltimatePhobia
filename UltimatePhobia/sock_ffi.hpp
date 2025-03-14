#pragma once

#include "ffi.hpp"
#include "misc_utils.hpp"

#include <winsock2.h>
#include <string>
#include <string_view>
#include <tuple>
#include <filesystem>

class SockFFI final : public FFI {
    std::string path;
    SOCKET clientSocket, listenSocket;

    void sendData(const void *, size_t);
    void sendString(std::string_view);
    template <typename T, unsigned size> void sendValue(T v) {
        static_assert(sizeof(T) == size);
        sendData(&v, size);
    }

    void receiveData(void *, size_t);
    std::string receiveString();
    template <typename T, unsigned size> T receiveValue() {
        static_assert(sizeof(T) == size);
        T v;
        receiveData(&v, size);
        return v;
    }

    void receiveValues() {}
    template <typename Arg0, typename... Args> void receiveValues(Arg0& arg0, Args&...args) {
        arg0 = receiveValue<Arg0, sizeof(Arg0)>();
        receiveValues(args...);
    }
    template <typename Tuple> Tuple receiveValuesTuple() {
        Tuple fres;
        std::apply([this](auto&...args) { receiveValues(args...); }, fres);
        return fres;
    }

    template <typename fncT> void doRpcCall(fncT *function) {
        static_assert(std::is_function<fncT>(), "Function to process RPC call for must be callable");
        using traits = function_traits<fncT>;
        const auto args = receiveValuesTuple<typename traits::argument_types>();
        if constexpr (!std::is_void_v<typename traits::return_type>) {
            const auto fres = std::apply([function](auto&...args) { return function(args...); }, args);
            sendValue<bool, 1>(true); // Function has finished executing
            sendValue<typename traits::return_type, sizeof(typename traits::return_type)>(fres);
        } else {
            std::apply([function](auto&...args) { function(args...); }, args);
            sendValue<bool, 1>(true); // Function has finished executing
        }
    }

    template <typename fncT> void doRpcCall(fncT *function, unsigned fncIdx, unsigned calledIdx) {
        if (fncIdx == calledIdx)
            doRpcCall(function);
    }

public:
    SockFFI(const std::filesystem::path& modPath);
    ~SockFFI();

    void simpleCall(const char *name);
};

template <> const char *SockFFI::receiveValue<const char *, sizeof(uintptr_t)>();
