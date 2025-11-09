#pragma once

#include <string_view>

std::string_view apply_name_deobfuscations(std::string_view name);
const char *apply_name_deobfuscations(const char *name);
