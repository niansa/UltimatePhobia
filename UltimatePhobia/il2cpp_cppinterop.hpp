#pragma once

#include <string>
#include <string_view>

struct System_String_o;

namespace Il2Cpp {
namespace CppInterop {
System_String_o *ToCsString(std::string_view);
std::string ToCppString(System_String_o *);
std::wstring_view ToCppWideString(System_String_o *);
void ToCString(System_String_o *, char *buf, size_t maxlen);

System_Type_o *GetType(std::string_view name, std::string_view assemblyName);
void *CreateInstance(System_Type_o *type);

inline static System_String_o *operator""_cs(const char *str, size_t len) { return ToCsString({str, len}); }
} // namespace CppInterop
} // namespace Il2Cpp
