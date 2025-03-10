#pragma once

#include "game_hook.hpp"

#include <string>
#include <string_view>
#include <optional>

std::string utf8Encode(std::wstring_view wstr);
std::string lastWinErrorString();
bool hookToggle(const char *description, std::optional<GameHook>& hook, bool& boolean, void *method, void *hookFnc);
bool hookToggle(const char *description, GameHookPool& hookPool, bool& boolean, void *method, void *hookFnc);
