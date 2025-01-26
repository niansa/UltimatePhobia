#pragma once

#include "global_state.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>


class Mod;

struct ModInfo {
    const std::string name;
    bool hidden = false;
    const std::function<std::unique_ptr<Mod> ()> create;
    const std::function<void ()> onAppStart = nullptr;
    std::unique_ptr<Mod> instance = nullptr;

    bool isLoaded() const {
        return instance != nullptr;
    }

    void load() {
        g.logger->info("Loading mod '{}'...", name);
        instance = create();
        g.logger->info("Mod '{}' loaded.", name);
    }
    void unload() {
        g.logger->info("Unloading mod '{}'...", name);
        instance.reset();
        g.logger->info("Mod '{}' unloaded.", name);
    }

    template<class ModT>
    ModT *get() {
        if (!isLoaded())
            load();
        return dynamic_cast<ModT*>(instance.get());
    }
};

class ModPanic {
    // This class isn't inherited from std::exception on purpose so it can't accidentally be caught through generic std::exception handlers
    ModInfo& mod;
    std::string msg;

public:
    ModPanic(ModInfo& mod, std::string_view msg) : mod(mod), msg(msg) {}
    ~ModPanic() {
        // Unload mod after exception has been handled
        mod.unload();
    }
    const char *where() const {
        return mod.name.c_str();
    }
    const char *what() const {
        return msg.c_str();
    }
};

class Mod {
public:
    Mod() {}
    virtual ~Mod() {}

    virtual void uiUpdate() {}
};
