#include "game_types.hpp"
#include "generated/il2cpp.hpp"
#include "misc_utils.hpp"

#include <string_view>


namespace GameTypes {
System_String_o *createCsString(std::string_view str) {
    return Il2Cpp::System::String::CreateString(nullptr, const_cast<signed char *>(reinterpret_cast<const signed char *>(str.data())), 0, str.size());
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
