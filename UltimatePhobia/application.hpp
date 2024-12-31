#pragma once

#include "mods/base.hpp"

#include <vector>


class Application {
    friend struct ApplicationHooks;

    std::vector<ModInfo*> mods;

    void init();
    void update();

public:
    Application();
    Application(const Application&) = delete;
    Application(Application&&) = delete;

    static void exit(int code);
    static bool isActive();
};
