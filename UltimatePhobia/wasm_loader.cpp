#include "wasm_loader.hpp"
#include "il2cpp_dynamic.hpp"
#include "il2cpp_cppinterop.hpp"
#include "global_state.hpp"
#include "wasm_interface.h"

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <map>
#include <optional>
#include <exception>
#include <stdexcept>
#include <simdjson.h>
#include <wasm3_cpp.h>



namespace WASMLoader {
namespace {
wasm3::wasm_environment env;
}


class WASMMod : public Mod {
    const std::string wasmPath;

    wasm3::wasm_runtime runtime;
    std::optional<wasm3::wasm_module> module;

public:
    WASMMod(const std::filesystem::path& wasmPath)
        : wasmPath(wasmPath.string())
        , runtime(env.new_runtime(1024)) {
        // Load WASM file
        std::ifstream wasmFile(wasmPath, std::ios::binary | std::ios::in);
        if (!wasmFile)
            throw std::runtime_error("Failed to open WASM file for reading");
        module = env.parse_module(wasmFile);
        runtime.load(*module);

        // Set up bindings
        using namespace WASMInterface;
        module->link_optional("*", "dropObject", dropObject);
        module->link_optional("*", "isValidObject", isValidObject);
        module->link_optional("*", "toCsString", toCsString);
        module->link_optional("*", "toCsStringWithLength", toCsStringWithLength);
        module->link_optional("*", "toCString", toCString);
        module->link_optional("*", "logTrace", logTrace);
        module->link_optional("*", "logDebug", logDebug);
        module->link_optional("*", "logInfo", logInfo);
        module->link_optional("*", "logWarn", logWarn);
        module->link_optional("*", "logError", logError);
        module->link_optional("*", "logCritical", logCritical);
        module->link_optional("*", "getMethodByIdentifier", getMethodByIdentifier);
        module->link_optional("*", "getMethodByAddress", getMethodByAddress);
        module->link_optional("*", "getMethodName", getMethodName);
        module->link_optional("*", "getMethodSignature", getMethodSignature);
        module->link_optional("*", "addArgI32", addArgI32);
        module->link_optional("*", "addArgI64", addArgI64);
        module->link_optional("*", "addArgFloat", addArgFloat);
        module->link_optional("*", "addArgDouble", addArgDouble);
        module->link_optional("*", "addArgObject", addArgObject);
        module->link_optional("*", "addArgNull", addArgNull);
        module->link_optional("*", "clearArgs", clearArgs);
        module->link_optional("*", "getValueI32", getValueI32);
        module->link_optional("*", "getValueI64", getValueI64);
        module->link_optional("*", "getValueFloat", getValueFloat);
        module->link_optional("*", "getValueDouble", getValueDouble);
        module->link_optional("*", "getValueObject", getValueObject);
        module->link_optional("*", "getCallError", getCallError);
        module->link_optional("*", "call", call);

        // Let module initialize itself
        runtime.find_function("onLoad").call();
    }
    ~WASMMod() {
        // Let module clean up after itself
        runtime.find_function("onUnload").call();
    }

    void uiUpdate() override {
        runtime.find_function("onUiUpdate").call();
    }
};


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

    // Create mod info
    return new ModInfo {
        std::string(json["name"].get_string().value()),
        false,
        [wasmPath, identifier = std::string(identifier)] () -> std::unique_ptr<Mod> {
            try {
                return std::make_unique<WASMMod>(wasmPath);
            } catch (const std::exception& e) {
                g.logger->error("Failed to load WASM module '{}': {}", identifier, e.what());
                return nullptr;
            }
        }
    };
}
}
