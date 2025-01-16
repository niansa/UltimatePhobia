#include "il2cpp_dynamic.hpp"
#include "global_state.hpp"
#include "safe_path.hpp"
#include "generated/il2cpp.hpp"

#include <chrono>
#include <filesystem>
#include <thread>
#include <mutex>
#include <cstdint>
#include <simdjson.h>



namespace Il2Cpp::Dynamic {
namespace {
std::vector<Method> methods;
std::mutex methods_mutex; // Intentionally used only very loosely
}


void init() {
    std::thread([] () {
        std::scoped_lock L(methods_mutex);

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
        static simdjson::ondemand::parser parser;
        static auto json = simdjson::padded_string::load(scriptJsonPath.string());
        const auto time_start = high_resolution_clock::now();
        auto scriptJson = parser.iterate(json);
        // Get functions
        unsigned errors = 0;
        for (auto functionJson : scriptJson["ScriptMethod"]) {
            try {
                Method m;
                m.address = functionJson["Address"].get_int64().value();
                m.name = functionJson["Name"];
                m.signature = functionJson["Signature"];
                m.typeSignature = functionJson["TypeSignature"];
                m.index = methods.size();
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
            if (getMethod(name).getFullAddress() != ptr)
                valid = false;
        };
        validate("void UnityEngine_Application__Quit (const MethodInfo* method);", Il2Cpp::UnityEngine::Application::Quit_getPtr());
        validate("Player$$Update", Il2Cpp::Player::Update_getPtr());
        validate("GhostAI$$Appear", Il2Cpp::GhostAI::Appear_getPtr());
        if (!valid)
            g.logger->warn("Loaded script.json file doesn't match script.json UltimatePhobia was compiled with! Expect serious issues.");
    }).detach();
}

bool isLoaded() {
    std::scoped_lock L(methods_mutex);
    return !methods.empty();
}

Method getMethod(std::string_view identifier, bool noError) {
    for (const Method& method : methods) {
        if (method.signature == identifier)
            return method;
    }

    for (const Method& method : methods) {
        if (method.name == identifier)
            return method;
    }

    if (!noError)
        g.logger->error("Failed to find method by identifier: {}", identifier);
    return {0};
}

Method getMethod(void *addr, bool noError) {
    for (Method method : methods)
        if (method.getFullAddress() == addr)
            return method;

    if (!noError)
        g.logger->error("Failed to find method by address: {}", addr);
    return {0};
}

Method getMethod(unsigned idx, bool noError) {
    if (idx > methods.size()) {
        if (!noError)
            g.logger->error("Failed to find method by index: {}", idx);
        return {0};
    }

    return methods[idx];
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
        if (method.name.find(identifier) != std::string_view::npos)
            fres.emplace_back(method);
    }

    // Finally try signature search
    for (const Method& method : methods) {
        if (isDup(method))
            continue;
        if (method.signature.find(identifier) != std::string_view::npos)
            fres.emplace_back(method);
    }

    return fres;
}

const std::vector<Method>& getMethods() {
    return methods;
}
}
