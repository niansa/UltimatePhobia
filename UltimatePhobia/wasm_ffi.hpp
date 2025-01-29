#pragma once

#include "ffi.hpp"

#include <optional>
#include <filesystem>
#include <wasm3_cpp.h>


class WASMFFI final : public FFI {
    wasm3::wasm_runtime runtime;
    std::optional<wasm3::wasm_module> module;

public:
    WASMFFI(const std::filesystem::path& modPath, unsigned memSize);

    void simpleCall(const char *name) override;
};
