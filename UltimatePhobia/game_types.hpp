#pragma once

#include <string>
#include <string_view>

struct System_String_o;


namespace GameTypes {
System_String_o *createCsString(std::string_view);
std::string toCppString(System_String_o *);
void toCString(System_String_o *, char *buf, size_t maxlen);
}
