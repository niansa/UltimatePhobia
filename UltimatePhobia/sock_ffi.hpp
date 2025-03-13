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
        sendData(&v, sizeof(v));
    }

    void receiveData(void *, size_t);
    std::string receiveString();
    template <typename T, unsigned size> void receiveValue(T& v) {
        static_assert(sizeof(T) == size);
        receiveData(&v, sizeof(v));
    }
    template <typename T, unsigned size> T receiveValue() {
        static_assert(sizeof(T) == size);
        T v;
        receiveData(&v, sizeof(v));
        return v;
    }
    template <> const char *receiveValue<const char *, sizeof(uintptr_t)>() {
        static std::string buf;
        buf = receiveString();
        return buf.c_str();
    }

    void receiveValues() {}
    template <typename Arg0, typename... Args> void receiveValues(Arg0& arg0, Args&...args) {
        receiveValue<Arg0, sizeof(Arg0)>();
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
        sendValue<bool, 1>(true); // Function has finished executing
        if constexpr (!std::is_void_v<typename traits::return_type>)
            sendValue<typename traits::return_type, sizeof(typename traits::return_type)>(
                std::apply([function](auto&...args) { return function(args...); }, args));
        else
            std::apply([function](auto&...args) { function(args...); }, args);
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
