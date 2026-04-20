#include "il2cpp_cppinterop.hpp"
#include "generated/il2cpp.hpp"
#include "misc_utils.hpp"

#include <format>
#include <string_view>
#include <spdlog/spdlog.h>

namespace Il2Cpp::CppInterop {
System_String_o *ToCsString(std::string_view str) {
    if (str == "<null>")
        return nullptr;
    return Il2Cpp::System::String::CreateString(nullptr, const_cast<System_SByte_o *>(reinterpret_cast<const System_SByte_o *>(str.data())), 0, str.size());
}

std::string ToCppString(System_String_o *str) {
    if (str == nullptr)
        return "<null>";

    return utf8Encode({reinterpret_cast<wchar_t *>(&str->fields.m_firstChar), static_cast<size_t>(str->fields.m_stringLength)});
}

std::wstring_view ToCppWideString(System_String_o *str) {
    if (str == nullptr)
        return L"<null>";

    return {reinterpret_cast<wchar_t *>(&str->fields.m_firstChar), static_cast<size_t>(str->fields.m_stringLength)};
}

void ToCString(System_String_o *str, char *buf, size_t maxlen) {
    const auto cppstr = ToCppString(str);
    memcpy(buf, cppstr.c_str(), std::max(cppstr.size() + 1, maxlen));
}

namespace {
System_Type_o *GetType(std::string_view name) {
    const auto fres = System::Type::GetType(CppInterop::ToCsString(name));
    if (!fres)
        g.logger->error("Failed to get type '{}'!", name);
    return fres;
}
} // namespace

System_Type_o *GetType(std::string_view name, std::string_view assemblyName) {
    return GetType(std::format("{}, {}, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null", name, assemblyName));
}

void *CreateInstance(System_Type_o *type) { return System::Activator::CreateInstance(type); }
} // namespace Il2Cpp::CppInterop
