#include "game_types.hpp"
#include "gamedata.hpp"
#include "misc_utils.hpp"

#include <string_view>


namespace GameTypes {
System_String_o *createCsString(std::string_view str) {
    const auto method = GameData::getMethod("System_String_o* System_String__CreateString (System_String_o* __this, int8_t* value, int32_t startIndex, int32_t length, const MethodInfo* method);");
    return method.getFunction<System_String_o *(System_String_o *, const char *, int32_t, int32_t, const MethodInfo *)>()(nullptr, str.data(), 0, str.size(), nullptr);
}

std::string toCppString(System_String_o *str) {
    if (str == nullptr)
        return {};

    return utf8Encode({reinterpret_cast<wchar_t*>(&str->fields.m_firstChar), static_cast<size_t>(str->fields.m_stringLength)});
}

void toCString(System_String_o *str, char *buf, size_t maxlen) {
    const auto cppstr = toCppString(str);
    memcpy(buf, cppstr.c_str(), std::max(cppstr.size()+1, maxlen));
}
}
