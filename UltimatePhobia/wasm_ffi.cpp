#include "wasm_ffi.hpp"
#include "ffi_interface.hpp"

#include <fstream>

namespace {
wasm3::wasm_environment env;
}

WASMFFI::WASMFFI(const std::filesystem::path& modPath, unsigned int memSize) : runtime(env.new_runtime(memSize)) {
    // Load WASM file
    std::ifstream wasmFile(modPath, std::ios::binary | std::ios::in);
    if (!wasmFile)
        throw std::runtime_error("Failed to open WASM file for reading");

    // Create runtime and load module
    module = env.parse_module(wasmFile);
    wasmFile.close();
    runtime.load(*module);

    // Set up bindings
#ifndef FFI_NOSTL
#define FFI_FUNCTION_LIST_ENTRY(return_type, fnc, arguments, ...) module->link_optional("*", #fnc, FFIInterface::fnc);
    FFI_FUNCTION_LIST
#undef FFI_FUNCTION_LIST_ENTRY
#endif

    // Call initialize
    std::optional<wasm3::wasm_function> init;
    try {
        init = runtime.find_function("_initialize");
    } catch (const wasm3::error&) {
    }
    if (init.has_value())
        init->call();
}

void WASMFFI::simpleCall(const char *name) { runtime.find_function(name).call(); }
