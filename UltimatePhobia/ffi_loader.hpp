#pragma once

#include "mods/base.hpp"
#include "ffi.hpp"

#include <string_view>
#include <filesystem>
#include <memory>

namespace FFILoader {
struct FFIModInternal;

class FFIMod : public Mod {
    ModInfo *const modInfo;
    std::unique_ptr<FFI> ffi;

public:
    FFIMod(const std::filesystem::path& base, std::string_view identifier, ModInfo *modInfo, unsigned memSize = 1024);
    ~FFIMod();
    FFIMod(const FFIMod&) = delete;
    FFIMod(FFIMod&&) = delete;

    void uiUpdate() override;

    void simpleCall(const char *name);

    static ModInfo *getCurrent();
    static void setCurrent(ModInfo *mod);
};

ModInfo *createModInfo(const std::filesystem::path& base, std::string_view identifier);
} // namespace FFILoader
