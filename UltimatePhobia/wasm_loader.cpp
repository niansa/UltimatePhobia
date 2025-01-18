#include "wasm_loader.hpp"
#include "global_state.hpp"
#include "wasm_interface.hpp"

#include <string>
#include <fstream>
#include <filesystem>
#include <optional>
#include <memory>
#include <exception>
#include <stdexcept>
#include <simdjson.h>
#include <wasm3_cpp.h>


namespace WASMLoader {
namespace {
wasm3::wasm_environment env;
ModInfo *currentMod;
}


struct WASMModInternal {
    wasm3::wasm_runtime runtime;
    std::optional<wasm3::wasm_module> module;
};


WASMMod::WASMMod(const std::filesystem::path& wasmPath, ModInfo *modInfo, unsigned memSize)
    : wasmPath(wasmPath.string()), modInfo(modInfo) {
    // Load WASM file
    std::ifstream wasmFile(wasmPath, std::ios::binary | std::ios::in);
    if (!wasmFile)
        throw std::runtime_error("Failed to open WASM file for reading");

    // Create runtime and load module
    i = new WASMModInternal {env.new_runtime(memSize), env.parse_module(wasmFile)};
    i->runtime.load(*i->module);

    // Set up bindings
    using namespace WASMInterface;
    i->module->link_optional("*", "dropObject", dropObject);
    i->module->link_optional("*", "isValidObject", isValidObject);
    i->module->link_optional("*", "toCsString", toCsString);
    i->module->link_optional("*", "toCsStringWithLength", toCsStringWithLength);
    i->module->link_optional("*", "toCString", toCString);
    i->module->link_optional("*", "logTrace", logTrace);
    i->module->link_optional("*", "logDebug", logDebug);
    i->module->link_optional("*", "logInfo", logInfo);
    i->module->link_optional("*", "logWarn", logWarn);
    i->module->link_optional("*", "logError", logError);
    i->module->link_optional("*", "logCritical", logCritical);
    i->module->link_optional("*", "getMethodByIdentifier", getMethodByIdentifier);
    i->module->link_optional("*", "getMethodByAddress", getMethodByAddress);
    i->module->link_optional("*", "getMethodName", getMethodName);
    i->module->link_optional("*", "getMethodSignature", getMethodSignature);
    i->module->link_optional("*", "addArgI32", addArgI32);
    i->module->link_optional("*", "addArgI64", addArgI64);
    i->module->link_optional("*", "addArgFloat", addArgFloat);
    i->module->link_optional("*", "addArgDouble", addArgDouble);
    i->module->link_optional("*", "addArgObject", addArgObject);
    i->module->link_optional("*", "addArgNull", addArgNull);
    i->module->link_optional("*", "clearArgs", clearArgs);
    i->module->link_optional("*", "getArgCount", getArgCount);
    i->module->link_optional("*", "moveArg", moveArg);
    i->module->link_optional("*", "getValueI32", getValueI32);
    i->module->link_optional("*", "getValueI64", getValueI64);
    i->module->link_optional("*", "getValueFloat", getValueFloat);
    i->module->link_optional("*", "getValueDouble", getValueDouble);
    i->module->link_optional("*", "getValueObject", getValueObject);
    i->module->link_optional("*", "getCallError", getCallError);
    i->module->link_optional("*", "call", call);
    i->module->link_optional("*", "hook", hook);
    i->module->link_optional("*", "unhook", unhook);
    i->module->link_optional("*", "getOriginal", getOriginal);

    // Call init
    std::optional<wasm3::wasm_function> init;
    try {
        init = i->runtime.find_function("_initialize");
    } catch (const wasm3::error&) {}
    if (init.has_value())
        init->call();

    // Let module initialize itself
    simpleCall("onLoad");
}
WASMMod::~WASMMod() {
    // Let module clean up after itself
    simpleCall("onUnload");

    delete i;
}

void WASMMod::uiUpdate() {
    simpleCall("onUiUpdate");
}

void WASMMod::simpleCall(const char *name) {
    currentMod = modInfo;
    i->runtime.find_function(name).call();
}

ModInfo *WASMMod::getCurrent() {
    return currentMod;
}


ModInfo *createModInfo(const std::filesystem::path& base, std::string_view identifier) {
    // Make sure JSON and WASM files both exist
    const auto jsonPath = base/fmt::format("{}.json", identifier),
               wasmPath = base/fmt::format("{}.wasm", identifier);
    (void)std::filesystem::file_size(jsonPath);
    (void)std::filesystem::file_size(wasmPath);

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
    auto fres = new ModInfo {
        std::string(json["name"].get_string().value()),
        false,
        [wasmPath, modInfoPtr, memSize, identifier = std::string(identifier)] () -> std::unique_ptr<Mod> {
            try {
                return std::make_unique<WASMMod>(wasmPath, *modInfoPtr, memSize);
            } catch (const std::exception& e) {
                g.logger->error("Failed to load WASM module '{}': {}", identifier, e.what());
                return nullptr;
            }
        }
    };
    *modInfoPtr = fres;
    return fres;
}
}
