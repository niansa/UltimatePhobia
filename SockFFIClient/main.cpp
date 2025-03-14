#include "socket.hpp"
#include "dlhandle.hpp"

#include <iostream>
#include <string_view>
#include <cstring>
#include <ffi_interface.hpp>

namespace {
// No point in using smart pointers here
Socket *socket;
Dlhandle *library;

void waitForCommand();

void sendFunctionIndex(size_t index) { socket->sendValue<uint8_t, 1>(index + 1); }

template <typename retT> retT waitRpcCallResult() {
    waitForCommand();
    if constexpr (!std::is_void<retT>())
        return socket->receiveValue<retT, sizeof(retT)>();
}
template <typename retT> retT doRpcCall_() { return waitRpcCallResult<retT>(); }
template <typename retT, typename argT, typename... argsT> retT doRpcCall_(argT arg, argsT... args) {
    socket->sendValue<argT, sizeof(argT)>(arg);
    return doRpcCall_<retT>(args...);
}
template <size_t fnc_index, typename retT, typename... argsT> retT doRpcCall(argsT... args) {
    sendFunctionIndex(fnc_index);
    return doRpcCall_<retT>(args...);
}

template <size_t fnc_index, typename retT, typename... argsT> retT prepareAndDoRpcCall(argsT... args) {
    const std::tuple argsTuple = {args...};

    // Handle known troublesome calls
    using enum FFIInterface::Functions;

    // Function redirects
    if constexpr (fnc_index == static_cast<size_t>(toCsString)) {
        const char *str = std::get<0>(argsTuple);
        return prepareAndDoRpcCall<static_cast<size_t>(toCsStringWithLength), retT, const char *, int>(str, strlen(str));
    }

    // Function handlers for special cases
    if constexpr (fnc_index == static_cast<size_t>(toCsStringWithLength)) {
        const char *str = std::get<0>(argsTuple);
        int len = std::get<1>(argsTuple);
        sendFunctionIndex(fnc_index);
        socket->sendString(std::string_view{str, static_cast<size_t>(len)});
        return waitRpcCallResult<retT>();
    }

    return doRpcCall<fnc_index, retT>(args...);
}

void waitForCommand() {
    const bool doExecute = !socket->receiveValue<bool, 1>();
    if (doExecute) {
        const auto fncName = socket->receiveString();
        const auto fnc = library->getFnc(fncName);
        if (fnc == nullptr) {
            socket->sendValue<bool, 1>(false);
            return;
        }
        socket->sendValue<bool, 1>(true);
        fnc();
        socket->sendValue<uint8_t, 1>(0);
    }
}
} // namespace

// FFI interface implementation
namespace FFIInterface {
#define FFI_FUNCTION_LIST_ENTRY(return_type, fnc, arguments, ...)                                                                                              \
    return_type fnc arguments {                                                                                                                                \
        constexpr auto _fnc_index = static_cast<size_t>(FFIInterface::Functions::fnc);                                                                         \
        return prepareAndDoRpcCall<_fnc_index, return_type>(__VA_ARGS__);                                                                                      \
    }
FFI_FUNCTION_LIST
#undef FFI_FUNCTION_LIST_ENTRY
} // namespace FFIInterface

int main(int argc, char **argv) {
    // Check args
    if (argc != 3) {
        std::cerr << "Bad usage!" << std::endl;
        std::cout << "Usage: " << argv[0] << " <socket> <implementation library>" << std::endl;
        return 1;
    }

    // Get args
    const char *socketPath = argv[1], *implementation = argv[2];

    // Load library
    library = new Dlhandle(implementation);

    // Create and connect socket client
    socket = new Socket(socketPath);
    if (!socket->connect())
        return 2;

    // Initalize exports
    const static FFIInterface::Exports exports;
    auto initImports = library->get<void(const FFIInterface::Exports *)>("initImports");
    if (!initImports)
        std::cerr << "DLL is missing 'initImports' function!" << std::endl;
    else
        initImports(&exports);

    // Main loop
    for (;;)
        waitForCommand();
}
