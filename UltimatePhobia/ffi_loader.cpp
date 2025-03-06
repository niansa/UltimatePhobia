#include "ffi_loader.hpp"
#include "global_state.hpp"
#include "wasm_ffi.hpp"
#include "dll_ffi.hpp"

#include <string>
#include <filesystem>
#include <memory>
#include <exception>
#include <stdexcept>
#include <simdjson.h>

namespace FFILoader {
namespace {
ModInfo *currentMod;
}

FFIMod::FFIMod(const std::filesystem::path& base, std::string_view identifier,
               ModInfo *modInfo, unsigned memSize)
    : modInfo(modInfo) {
    const auto getPath = [&](const char *extension) {
        return base / fmt::format("{}.{}", identifier, extension);
    };

    const auto wasmPath = getPath("wasm"), dllPath = getPath("dll");

    if (std::filesystem::exists(wasmPath))
        ffi = std::make_unique<WASMFFI>(wasmPath, memSize);
    else if (std::filesystem::exists(dllPath))
        ffi = std::make_unique<DLLFFI>(dllPath);
    else
        throw std::runtime_error("No valid FFI module found");

    // Let module initialize itself
    simpleCall("onLoad");
}
FFIMod::~FFIMod() {
    // Let module clean up after itself
    simpleCall("onUnload");
}

void FFIMod::uiUpdate() { simpleCall("onUiUpdate"); }

void FFIMod::simpleCall(const char *name) {
    currentMod = modInfo;
    ffi->simpleCall(name);
}

ModInfo *FFIMod::getCurrent() { return currentMod; }

void FFIMod::setCurrent(ModInfo *mod) { currentMod = mod; }

ModInfo *createModInfo(const std::filesystem::path& base,
                       std::string_view identifier) {
    // Make sure JSON and WASM files both exist
    const auto jsonPath = base / fmt::format("{}.json", identifier);
    (void)std::filesystem::file_size(jsonPath);

    // Load info from JSON
    simdjson::ondemand::parser parser;
    auto jsonData = simdjson::padded_string::load(jsonPath.string());
    auto json = parser.iterate(jsonData);

    // Get memory size
    unsigned memSize = 1024;
    auto memSizeJson = json["mem_size"];
    if (memSizeJson.error() == simdjson::SUCCESS)
        memSize = memSizeJson.get_int64().value();

    // Create mod info
    auto modInfoPtr = std::make_shared<ModInfo *>();
    auto fres = new ModInfo{
        std::string(json["name"].get_string().value()), false,
        [base, modInfoPtr, memSize,
         identifier = std::string(identifier)]() -> std::unique_ptr<Mod> {
            try {
                return std::make_unique<FFIMod>(base, identifier, *modInfoPtr,
                                                memSize);
            } catch (const std::exception& e) {
                g.logger->error("Failed to load FFI module '{}': {}",
                                identifier, e.what());
                return nullptr;
            }
        }};
    *modInfoPtr = fres;
    return fres;
}
} // namespace FFILoader
