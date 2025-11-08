#include "sock_ffi.hpp" // Has to be included first to avoid Microsoft stupidities surrounding winsock2.h
#include "ffi_loader.hpp"
#include "global_state.hpp"
#include "wasm_ffi.hpp"
#include "dll_ffi.hpp"
#include "elf_ffi.hpp"
#include "json.hpp"

#include <string>
#include <filesystem>
#include <fstream>
#include <memory>
#include <exception>
#include <stdexcept>

namespace FFILoader {
namespace {
ModInfo *currentMod;
}

FFIMod::FFIMod(const std::filesystem::path& base, std::string_view identifier, ModInfo *modInfo, unsigned memSize) : modInfo(modInfo) {
    const auto getPath = [&](const char *extension) { return base / fmt::format("{}.{}", identifier, extension); };

    const auto wasmPath = getPath("wasm"), dllPath = getPath("dll"), elfPath = getPath("so"), sockPath = getPath("socki");

    if (std::filesystem::exists(wasmPath))
        ffi = std::make_unique<WASMFFI>(wasmPath, memSize);
    else if (std::filesystem::exists(dllPath))
        ffi = std::make_unique<DLLFFI>(dllPath);
    else if (std::filesystem::exists(elfPath))
        ffi = std::make_unique<ELFFFI>(elfPath);
    else if (std::filesystem::exists(sockPath))
        ffi = std::make_unique<SockFFI>(sockPath);
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

ModInfo *createModInfo(const std::filesystem::path& base, std::string_view identifier) {
    // Make sure JSON and WASM files both exist
    const auto jsonPath = base / fmt::format("{}.json", identifier);
    (void)std::filesystem::file_size(jsonPath);

    // Load info from JSON
    auto json = nlohmann::json::parse(std::ifstream(jsonPath));

    // Get memory size
    unsigned memSize = 1024;
    auto memSizeJson = json["mem_size"];
    if (memSizeJson.is_number())
        memSize = memSizeJson;

    // Create mod info
    auto modInfoPtr = std::make_shared<ModInfo *>();
    auto nameJson = json["name"];
    auto fres = new ModInfo{nameJson.is_string() ? nameJson : "<bad modinfo name>", false,
                            [base, modInfoPtr, memSize, identifier = std::string(identifier)]() -> std::unique_ptr<Mod> {
                                try {
                                    return std::make_unique<FFIMod>(base, identifier, *modInfoPtr, memSize);
                                } catch (const std::exception& e) {
                                    g.logger->error("Failed to load FFI module '{}': {}", identifier, e.what());
                                    return nullptr;
                                }
                            }};
    *modInfoPtr = fres;
    return fres;
}
} // namespace FFILoader
