#pragma once

#include "mods/base.hpp"

#include <string>
#include <string_view>
#include <filesystem>


namespace WASMLoader {
struct WASMModInternal;

class WASMMod : public Mod {
    ModInfo *const modInfo;
    const std::string wasmPath;

    WASMModInternal *i;

public:
    WASMMod(const std::filesystem::path& wasmPath, ModInfo *modInfo, unsigned memSize = 1024);
    ~WASMMod();
    WASMMod(const WASMMod&) = delete;
    WASMMod(WASMMod&&) = delete;

    void uiUpdate() override;

    void simpleCall(const char *name);

    static ModInfo *getCurrent();
};

ModInfo *createModInfo(const std::filesystem::path& base, std::string_view identifier);
}
