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

std::string lastWinErrorString() {
    auto errorMessageID = GetLastError();
    if (errorMessageID == 0) {
        return "No error";
    }
    LPSTR messageBuffer = nullptr;
    auto size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    while (message.back() == '\n' || message.back() == '\r')
        message.pop_back();
    return message;
}

bool hookToggle(const char *description, std::optional<GameHook>& hook, bool& boolean, void *method, void *hookFnc) {
    if (ImGui::Checkbox(description, &boolean)) {
        if (boolean) {
            auto hook = GameHook::safeCreate(method, hookFnc);
            if (!hook.has_value()) {
                boolean = false;
                return false;
            }
            hook.emplace(GameHook(std::move(*hook)));
        } else {
            hook.reset();
        }
        return true;
    }
    return false;
}

bool hookToggle(const char *description, GameHookPool& hookPool, bool& boolean, void *method, void *hookFnc) {
    if (ImGui::Checkbox(description, &boolean)) {
        if (boolean)
            boolean = hookPool.add(method, hookFnc) != nullptr;
        else
            hookPool.remove(hookPool.get(method));
        return true;
    }
    return false;
}
