#include "il2cpp_dynamic.hpp"
#include "global_state.hpp"
#include "safe_path.hpp"
#include "generated/il2cpp.hpp"

#include <chrono>
#include <filesystem>
#include <cstdint>
#include <simdjson.h>



namespace Il2Cpp::Dynamic {
namespace {
std::vector<Method> methods;

inline void *calculateAddress(void *addr) {
    return reinterpret_cast<uint8_t *>(g.base) + reinterpret_cast<intptr_t>(addr);
}
}


void init() {
    using std::chrono::high_resolution_clock;
    const auto scriptJsonPath = SafePath::get()/"script.json";

    // Make sure file exists and get info
    unsigned scriptJsonSize;
    try {
        scriptJsonSize = std::filesystem::file_size(scriptJsonPath);
    } catch (...) {
        return;
    }

    // Start processing file
    g.logger->info("Processing script.json file for dynamic reflection...");
    methods.clear();
    const auto time_start = high_resolution_clock::now();
    static simdjson::ondemand::parser parser;
    static auto json = simdjson::padded_string::load(scriptJsonPath.string());
    auto scriptJson = parser.iterate(json);
    // Get functions
    unsigned errors = 0;
    for (auto functionJson : scriptJson["ScriptMethod"]) {
        try {
            Method m;
            m.address = reinterpret_cast<void *>(functionJson["Address"].get_int64().value());
            m.name = functionJson["Name"];
            m.signature = functionJson["Signature"];
            m.typeSignature = functionJson["TypeSignature"];
            methods.emplace_back(std::move(m));
        } catch (...) {
            ++errors;
        }
    }
    const auto time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - time_start).count();

    // Log out function count
    if (errors) {
        g.logger->info("Processed {} methods and failed on {} function from script.json file", methods.size(), errors);
    } else {
        g.logger->info("Processed {} methods from script.json file", methods.size());
    }
    g.logger->info("Processing script.json took {} ms ({} MB/s)", time_taken, unsigned((double(scriptJsonSize)*0.000001) / (double(time_taken)*0.001) + 0.5));

    // Validate some pointers
    bool valid = true;
    const auto validate = [&valid] (std::string_view name, void *ptr) {
        if (getMethod(name).address != ptr)
            valid = false;
    };
    validate("void UnityEngine_Application__Quit (const MethodInfo* method);", Il2Cpp::UnityEngine::Application::Quit_getPtr());
    validate("Player$$Update", Il2Cpp::Player::Update_getPtr());
    validate("GhostAI$$Appear", Il2Cpp::GhostAI::Appear_getPtr());
    if (!valid)
        g.logger->warn("Loaded script.json file doesn't match script.json UltimatePhobia was compiled with! Expect serious issues.");
}

bool isLoaded() {
    return !methods.empty();
}

Method getMethod(std::string_view identifier, bool noError) {
    for (const Method& method : methods) {
        if (method.signature == identifier) {
            Method fres(method);
            fres.address = calculateAddress(fres.address);
            return fres;
        }
    }

    for (const Method& method : methods) {
        if (method.name == identifier) {
            Method fres(method);
            fres.address = calculateAddress(fres.address);
            return fres;
        }
    }

    if (!noError)
        g.logger->error("Failed to find method by identifier: {}", identifier);
    return {nullptr};
}

Method getMethod(void *addr, bool noError) {
    for (Method method : methods) {
        method.address = calculateAddress(method.address);
        if (method.address == addr) {
            return method;
        }
    }

    if (!noError)
        g.logger->error("Failed to find method by address: {}", addr);
    return {nullptr};
}

std::vector<Method> searchMethods(std::string_view identifier) {
    std::vector<Method> fres;

    const auto isDup = [&fres] (const Method& method) {
        for (const auto& other_method : fres)
            if (method.signature == other_method.signature)
                return true;
        return false;
    };

    // Try exact search first
    {
        const auto result = getMethod(identifier, true);
        if (result.isValid())
            fres.emplace_back(std::move(result));
    }

    // Then try name search
    for (const Method& method : methods) {
        if (isDup(method))
            continue;
        if (method.name.find(identifier) != std::string_view::npos) {
            Method result(method);
            result.address = calculateAddress(result.address);
            fres.emplace_back(std::move(result));
        }
    }

    // Finally try signature search
    for (const Method& method : methods) {
        if (isDup(method))
            continue;
        if (method.signature.find(identifier) != std::string_view::npos) {
            Method result(method);
            result.address = calculateAddress(result.address);
            fres.emplace_back(std::move(result));
        }
    }

    return fres;
}
}
