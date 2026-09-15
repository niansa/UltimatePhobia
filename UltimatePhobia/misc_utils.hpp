#pragma once

#include "game_hook.hpp"

#include <string>
#include <string_view>
#include <optional>

std::string utf8Encode(std::wstring_view wstr);
std::wstring utf8Decode(std::string_view str);
bool isWordCharacter(char32_t codepoint);
void appendCodepointUtf8(std::string& output, char32_t codepoint);
std::string utf32ToUtf8(std::u32string_view input);
std::u32string utf16ToUtf32(std::wstring_view input);
std::u32string utf8ToUtf32(std::string_view input);
bool isWideCodepoint(char32_t codepoint);
int codepointWidth(char32_t codepoint);
std::size_t displayWidth(std::u32string_view text, std::size_t characterCount);

std::string lastWinErrorString();
bool hookToggle(const char *description, std::optional<GameHook>& hook, bool& boolean, void *method, void *hookFnc);
bool hookToggle(const char *description, GameHookPool& hookPool, bool& boolean, void *method, void *hookFnc);

template <typename T> struct function_traits;

template <typename R, typename... Args> struct function_traits<R(Args...)> {
    using return_type = R;
    using argument_types = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
};
