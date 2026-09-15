#include "misc_utils.hpp"
#include "game_hook.hpp"
#include "il2cpp_dynamic.hpp"

#include <string>
#include <string_view>
#include <optional>
#include <cstdint>
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
std::wstring utf8Decode(std::string_view str) {
    if (str.empty())
        return {};

    int len = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
    std::wstring wstrTo(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], len);
    return wstrTo;
}

bool isWordCharacter(char32_t codepoint) {
    return (codepoint >= U'a' && codepoint <= U'z') || (codepoint >= U'A' && codepoint <= U'Z') || (codepoint >= U'0' && codepoint <= U'9') ||
           codepoint == U'_';
}

void appendCodepointUtf8(std::string& output, char32_t codepoint) {
    if (codepoint <= 0x7F) {
        output.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
            codepoint = 0xFFFD;

        output.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0x10FFFF) {
        output.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        appendCodepointUtf8(output, 0xFFFD);
    }
}

std::string utf32ToUtf8(std::u32string_view input) {
    std::string output;
    output.reserve(input.size());

    for (const char32_t codepoint : input)
        appendCodepointUtf8(output, codepoint);

    return output;
}

std::u32string utf16ToUtf32(std::wstring_view input) {
    std::u32string output;
    output.reserve(input.size());

    for (std::size_t i = 0; i < input.size(); ++i) {
        const std::uint32_t first = static_cast<std::uint16_t>(input[i]);

        if (first >= 0xD800 && first <= 0xDBFF && i + 1 < input.size()) {
            const std::uint32_t second = static_cast<std::uint16_t>(input[i + 1]);

            if (second >= 0xDC00 && second <= 0xDFFF) {
                output.push_back(static_cast<char32_t>(0x10000 + ((first - 0xD800) << 10) + (second - 0xDC00)));

                ++i;
                continue;
            }
        }

        if (first >= 0xD800 && first <= 0xDFFF)
            output.push_back(0xFFFD);
        else
            output.push_back(static_cast<char32_t>(first));
    }

    return output;
}

std::u32string utf8ToUtf32(std::string_view input) {
    std::u32string output;
    output.reserve(input.size());

    std::size_t offset = 0;

    while (offset < input.size()) {
        const auto first = static_cast<unsigned char>(input[offset]);

        char32_t codepoint = 0;
        std::size_t continuationCount = 0;

        if (first <= 0x7F) {
            codepoint = first;
        } else if ((first & 0xE0) == 0xC0) {
            codepoint = first & 0x1F;
            continuationCount = 1;
        } else if ((first & 0xF0) == 0xE0) {
            codepoint = first & 0x0F;
            continuationCount = 2;
        } else if ((first & 0xF8) == 0xF0) {
            codepoint = first & 0x07;
            continuationCount = 3;
        } else {
            output.push_back(0xFFFD);
            ++offset;
            continue;
        }

        if (offset + continuationCount >= input.size()) {
            output.push_back(0xFFFD);
            break;
        }

        bool valid = true;

        for (std::size_t i = 1; i <= continuationCount; ++i) {
            const auto continuation = static_cast<unsigned char>(input[offset + i]);

            if ((continuation & 0xC0) != 0x80) {
                valid = false;
                break;
            }

            codepoint = (codepoint << 6) | static_cast<char32_t>(continuation & 0x3F);
        }

        const bool overlong =
            (continuationCount == 1 && codepoint < 0x80) || (continuationCount == 2 && codepoint < 0x800) || (continuationCount == 3 && codepoint < 0x10000);

        if (!valid || overlong || codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {

            output.push_back(0xFFFD);
            ++offset;
            continue;
        }

        output.push_back(codepoint);
        offset += continuationCount + 1;
    }

    return output;
}

bool isWideCodepoint(char32_t codepoint) {
    return (codepoint >= 0x1100 && codepoint <= 0x115F) || (codepoint >= 0x2329 && codepoint <= 0x232A) || (codepoint >= 0x2E80 && codepoint <= 0xA4CF) ||
           (codepoint >= 0xAC00 && codepoint <= 0xD7A3) || (codepoint >= 0xF900 && codepoint <= 0xFAFF) || (codepoint >= 0xFE10 && codepoint <= 0xFE19) ||
           (codepoint >= 0xFE30 && codepoint <= 0xFE6F) || (codepoint >= 0xFF00 && codepoint <= 0xFF60) || (codepoint >= 0xFFE0 && codepoint <= 0xFFE6) ||
           (codepoint >= 0x1F300 && codepoint <= 0x1FAFF) || (codepoint >= 0x20000 && codepoint <= 0x3FFFD);
}

int codepointWidth(char32_t codepoint) {
    if (codepoint == 0)
        return 0;

    // Common combining character ranges.
    if ((codepoint >= 0x0300 && codepoint <= 0x036F) || (codepoint >= 0x1AB0 && codepoint <= 0x1AFF) || (codepoint >= 0x1DC0 && codepoint <= 0x1DFF) ||
        (codepoint >= 0x20D0 && codepoint <= 0x20FF) || (codepoint >= 0xFE20 && codepoint <= 0xFE2F)) {
        return 0;
    }

    return isWideCodepoint(codepoint) ? 2 : 1;
}

std::size_t displayWidth(std::u32string_view text, std::size_t characterCount) {
    characterCount = std::min(characterCount, text.size());

    std::size_t width = 0;

    for (std::size_t i = 0; i < characterCount; ++i)
        width += static_cast<std::size_t>(std::max(codepointWidth(text[i]), 0));

    return width;
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
