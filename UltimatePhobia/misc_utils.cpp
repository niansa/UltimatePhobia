#include "misc_utils.hpp"

#include <string>
#include <string_view>
#include <windows.h>


std::string utf8Encode(std::wstring_view wstr) {
    if (wstr.empty())
        return {};

    int len = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], len, nullptr, nullptr);
    return strTo;
}
