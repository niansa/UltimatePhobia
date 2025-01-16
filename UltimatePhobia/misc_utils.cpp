#include "misc_utils.hpp"
#include "game_hook.hpp"
#include "il2cpp_dynamic.hpp"

#include <string>
#include <string_view>
#include <optional>
#include <windows.h>
#include <imgui.h>


std::string utf8Encode(std::wstring_view wstr) {
    if (wstr.empty())
        return {};

    int len = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], len, nullptr, nullptr);
    return strTo;
}

void hookToggle(const char *description, std::optional<GameHook>& hook, bool& boolean, void *method, void *hookFnc) {
    if (ImGui::Checkbox(description, &boolean)) {
        if (boolean)
            hook.emplace(method, hookFnc);
        else
            hook.reset();
    }
}
