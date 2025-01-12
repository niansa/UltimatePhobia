#include "tracer.hpp"
#include "gamedata.hpp"
#include "global_state.hpp"

#include <string>
#include <imgui.h>



extern "C"
void *tracerModHook(void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h) {
    static int depth = 0;
    if (depth < 0)
        depth = 0;

    const auto method = GameData::getMethod(GameHook::getTrampolineCaller());
    auto& hook = tracerInfo.get<Tracer>()->getHook(method.signature);
    tracerInfo.get<Tracer>()->log(fmt::format("{}{}<{}>(...)\n", std::string(depth, '>'), method.name, GameHook::getTrampolineCaller()));

    GameHookRelease GHR(hook);
    ++depth;
    const auto fres = reinterpret_cast<decltype(tracerModHook)*>(GameHook::getTrampolineCaller())(a, b, c, d, e, f, g, h);
    --depth;
    return fres;
}
GAMEHOOK_TRAMPOLINE(tracerModHook)


void Tracer::uiUpdate() {
    using namespace ImGui;
    Begin("Function Tracer");
    // Contents
    TextUnformatted("Function search");
    InputText("Name", searchStringBuffer, IM_ARRAYSIZE(searchStringBuffer));
    // Get search string as C++ string
    std::string_view searchString{searchStringBuffer, strlen(searchStringBuffer)};
    // Show results list if search string is long enough
    if (searchString.size() > 3) {
        // Show all results
        BeginChild("Search results", ImVec2(0, GetFontSize() * 20.0f), true);
        for (const GameData::Method& method : GameData::searchMethods(searchString)) {
            HookButton(method.signature);
        }
        EndChild();
    }
    // Show list of enabled hooks
    TextUnformatted("Enabled hooks");
    BeginChild("Enabled hooks", ImVec2(0, GetFontSize() * 20.0f), true);
    for (auto& [signature, hook] : hooks) {
        HookButton(signature, true);
    }
    EndChild();
    // Show tracer log
    TextUnformatted("Tracer log");
    BeginChild("Tracer log", ImVec2(0, GetFontSize() * 20.0f), true);
    TextUnformatted(tracerLog.c_str());
    if (logAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        SetScrollHereY(1.0f);
    }
    EndChild();
    // Tracer log controls
    if (Button("Clear")) {
        tracerLog.clear();
    }
    SameLine();
    Checkbox("Autoscroll", &logAutoScroll);
    // Window end
    End();
}

void Tracer::HookButton(std::string_view signature, bool isDefinitelyHooked) {
    bool hooked = isDefinitelyHooked;
    if (!hooked)
        hooked = hooks.find(signature) != hooks.end();

    if (ImGui::Button((fmt::format("{} {}", hooked ? '-' : '+', signature)).c_str())) {
        const auto method = GameData::getMethod(signature);
        if (!hooked)
            hooks.emplace(signature, std::make_unique<GameHook>(method.address, hookTrampoline_tracerModHook, true));
        else
            hooks.erase(hooks.find(signature));
        log(fmt::format(" {} {} <{}>\n", hooked?'-':'+', method.name, method.address));
    }
}


ModInfo tracerInfo {
    "Tracer",
    false,
    [] () {return std::make_unique<Tracer>();}
};
