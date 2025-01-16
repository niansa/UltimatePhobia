#pragma once

#include "global_state.hpp"

#include <string>
#include <functional>
#include <memory>


class Mod {
public:
    Mod() {}
    virtual ~Mod() {}

    virtual void uiUpdate() {}
};

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
