#include "socket.hpp"
#include "dlhandle.hpp"

#include <iostream>
#include <string_view>
#include <filesystem>
#include <fstream>
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
    if constexpr (!std::is_void<retT>()) {
        const retT fres = socket->receiveValue<retT, sizeof(retT)>();
        return fres;
    }
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
    if constexpr (fnc_index == static_cast<size_t>(ImGuiCheckbox)) {
        const char *label = std::get<0>(argsTuple);
        bool *v = std::get<1>(argsTuple);
        *v = prepareAndDoRpcCall<static_cast<size_t>(ImGuiCheckbox2), FFIInterface::WIBool, const char *, FFIInterface::WIBool>(label, *v);
        return;
    }

    // Function handlers for special cases
    if constexpr (fnc_index == static_cast<size_t>(toCsStringWithLength)) {
        const char *str = std::get<0>(argsTuple);
        int len = std::get<1>(argsTuple);
        sendFunctionIndex(fnc_index);
        socket->sendString(std::string_view{str, static_cast<size_t>(len)});
        return waitRpcCallResult<retT>();
    }
    if constexpr (fnc_index == static_cast<size_t>(toCString)) {
        FFIInterface::ObjectHandle csString = std::get<0>(argsTuple);
        char *buf = std::get<1>(argsTuple);
        int maxlen = std::get<2>(argsTuple);
        sendFunctionIndex(fnc_index);
        socket->sendValue<decltype(csString), sizeof(csString)>(csString);
        socket->sendValue<decltype(maxlen), sizeof(maxlen)>(maxlen);
        waitRpcCallResult<retT>();
        const auto cString = socket->receiveString();
        memcpy(buf, cString.data(), std::min<size_t>(cString.size(), maxlen));
        return;
    }
    if constexpr (fnc_index == static_cast<size_t>(copyArrayBytes)) {
        FFIInterface::ObjectHandle array = std::get<0>(argsTuple);
        int32_t offset = std::get<1>(argsTuple);
        int32_t length = std::get<2>(argsTuple);
        void *to = std::get<3>(argsTuple);
        sendFunctionIndex(fnc_index);
        socket->sendValue<decltype(array), sizeof(array)>(array);
        socket->sendValue<decltype(offset), sizeof(offset)>(offset);
        socket->sendValue<decltype(length), sizeof(length)>(length);
        waitRpcCallResult<retT>();
        socket->receiveData(to, length);
        return;
    }

    return doRpcCall<fnc_index, retT>(args...);
}

void waitForCommand() {
beginning:
    const bool doExecute = !socket->receiveValue<bool, 1>();
    if (doExecute) {
        const auto fncName = socket->receiveString();
        std::cout << "Attempting to execute " << fncName << "..." << std::endl;
        const auto fnc = library->getFnc(fncName);
        if (fnc == nullptr) {
            std::cout << "Could not find " << fncName << "!" << std::endl;
            socket->sendValue<bool, 1>(false);
            return;
        }
        socket->sendValue<bool, 1>(true);
        fnc();
        std::cout << "Finished executing " << fncName << '!' << std::endl;
        socket->sendValue<uint8_t, 1>(0);
        goto beginning; // Instead of tail call to self to avoid stack overflow in debug build
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
    std::cout << "Loading library " << implementation << "..." << std::endl;
    library = new Dlhandle(implementation);

    // Create and connect socket client
    socket = new Socket(socketPath);
    if (!socket->connect())
        return 2;

    // Initalize exports
    std::cout << "Initializing imports..." << std::endl;
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
