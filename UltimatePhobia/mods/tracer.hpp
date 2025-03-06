#pragma once

#include "base.hpp"
#include "game_hook.hpp"

#include <string>
#include <string_view>
#include <memory>
#include <map>

class Tracer final : public Mod {
    std::string tracerLog;
    char searchStringBuffer[128] = {'\0'};
    bool logAutoScroll = true;
    std::map<std::string_view, std::unique_ptr<GameHook>> hooks;

    void HookButton(std::string_view signature,
                    bool isDefinitelyHooked = false);

public:
    void uiUpdate() override;

    void log(std::string_view msg) { tracerLog.append(msg); }

    GameHook& getHook(std::string_view signature) { return *hooks[signature]; }
};

extern ModInfo tracerInfo;
