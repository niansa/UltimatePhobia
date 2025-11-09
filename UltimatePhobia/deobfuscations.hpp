#pragma once

#include <string_view>

std::string_view apply_name_deobfuscations(std::string_view name);
const char *apply_name_deobfuscations(const char *name);

std::string_view apply_name_obfuscations(std::string_view name);
const char *apply_name_obfuscations(const char *name);
