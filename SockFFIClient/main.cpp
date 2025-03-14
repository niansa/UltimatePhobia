#include "socket.hpp"
#include "utils.hpp"
#include "dlhandle.hpp"

#include <iostream>
#include <ffi_interface.hpp>

namespace {
// No point in using smart pointers here
Socket *socket;
Dlhandle *library;

void waitForCommand();

template <typename retT> retT doRpcCall_() {
    waitForCommand();
    if constexpr (!std::is_void<retT>())
        return socket->receiveValue<retT, sizeof(retT)>();
}
template <typename retT, typename argT, typename... argsT> retT doRpcCall_(argT arg, argsT... args) {
    socket->sendValue<argT, sizeof(argT)>(arg);
    return doRpcCall_<retT>(args...);
}
template <typename retT, typename... argsT> retT doRpcCall(unsigned index, argsT... args) {
    socket->sendValue<uint8_t, 1>(index);
    return doRpcCall_<retT>(args...);
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
        const auto _fnc_index = tuple_find(FFIInterface::functions, &fnc);                                                                                     \
        if (!_fnc_index.has_value()) {                                                                                                                         \
            std::cerr << "Bad FFI function index called" << std::endl;                                                                                         \
            ::abort();                                                                                                                                         \
        }                                                                                                                                                      \
        return doRpcCall<return_type>((*_fnc_index + 1) /*TODO*/ __VA_ARGS__);                                                                                 \
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
