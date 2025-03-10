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
    module->link_optional("*", "dropObject", FFIInterface::dropObject);
    module->link_optional("*", "isValidObject", FFIInterface::isValidObject);
    module->link_optional("*", "toCsString", FFIInterface::toCsString);
    module->link_optional("*", "toCsStringWithLength", FFIInterface::toCsStringWithLength);
    module->link_optional("*", "toCString", FFIInterface::toCString);
    module->link_optional("*", "logTrace", FFIInterface::logTrace);
    module->link_optional("*", "logDebug", FFIInterface::logDebug);
    module->link_optional("*", "logInfo", FFIInterface::logInfo);
    module->link_optional("*", "logWarn", FFIInterface::logWarn);
    module->link_optional("*", "logError", FFIInterface::logError);
    module->link_optional("*", "logCritical", FFIInterface::logCritical);
    module->link_optional("*", "getMethodByIdentifier", FFIInterface::getMethodByIdentifier);
    module->link_optional("*", "getMethodByAddress", FFIInterface::getMethodByAddress);
    module->link_optional("*", "getMethodName", FFIInterface::getMethodName);
    module->link_optional("*", "getMethodSignature", FFIInterface::getMethodSignature);
    module->link_optional("*", "addArgI32", FFIInterface::addArgI32);
    module->link_optional("*", "addArgI64", FFIInterface::addArgI64);
    module->link_optional("*", "addArgFloat", FFIInterface::addArgFloat);
    module->link_optional("*", "addArgDouble", FFIInterface::addArgDouble);
    module->link_optional("*", "addArgObject", FFIInterface::addArgObject);
    module->link_optional("*", "addArgNull", FFIInterface::addArgNull);
    module->link_optional("*", "clearArgs", FFIInterface::clearArgs);
    module->link_optional("*", "getArgCount", FFIInterface::getArgCount);
    module->link_optional("*", "moveArg", FFIInterface::moveArg);
    module->link_optional("*", "getValueI32", FFIInterface::getValueI32);
    module->link_optional("*", "getValueI64", FFIInterface::getValueI64);
    module->link_optional("*", "getValueFloat", FFIInterface::getValueFloat);
    module->link_optional("*", "getValueDouble", FFIInterface::getValueDouble);
    module->link_optional("*", "getValueObject", FFIInterface::getValueObject);
    module->link_optional("*", "getCallError", FFIInterface::getCallError);
    module->link_optional("*", "call", FFIInterface::call);
    module->link_optional("*", "hook", FFIInterface::hook);
    module->link_optional("*", "unhook", FFIInterface::unhook);
    module->link_optional("*", "getOriginal", FFIInterface::getOriginal);
    module->link_optional("*", "ImGuiBegin", FFIInterface::ImGuiBegin);
    module->link_optional("*", "ImGuiEnd", FFIInterface::ImGuiEnd);
    module->link_optional("*", "ImGuiText", FFIInterface::ImGuiText);
    module->link_optional("*", "ImGuiCheckbox", FFIInterface::ImGuiCheckbox);
    module->link_optional("*", "ImGuiButton", FFIInterface::ImGuiButton);
    module->link_optional("*", "ImGuiSeparator", FFIInterface::ImGuiSeparator);
    module->link_optional("*", "ImGuiSeparatorText", FFIInterface::ImGuiSeparatorText);
    module->link_optional("*", "abort", FFIInterface::abort);

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
