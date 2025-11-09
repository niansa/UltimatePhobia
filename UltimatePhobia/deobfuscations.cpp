#include "deobfuscations.hpp"

#include <string_view>

#define OBFUSCATION_MAP OBFUSCATED_STRING("\u0a68\u0a69\u0a6f\u0a67\u0a68\u0a67\u0a70\u0a67\u0a69\u0a73\u0a67", "Use")

template <typename StrT> StrT apply_name_deobfuscations_any(StrT name) {
#define OBFUSCATED_STRING(obfuscated, actual)                                                                                                                  \
    if (std::string_view(name) == obfuscated)                                                                                                                  \
        name = actual;
    OBFUSCATION_MAP
#undef OBFUSCATED_STRING
    return name;
}
std::string_view apply_name_deobfuscations(std::string_view name) { return apply_name_deobfuscations_any(name); }
const char *apply_name_deobfuscations(const char *name) { return apply_name_deobfuscations_any(name); }

template <typename StrT> StrT apply_name_obfuscations_any(StrT name) {
#define OBFUSCATED_STRING(obfuscated, actual)                                                                                                                  \
    if (std::string_view(name) == actual)                                                                                                                      \
        name = obfuscated;
    OBFUSCATION_MAP
#undef OBFUSCATED_STRING
    return name;
}
std::string_view apply_name_obfuscations(std::string_view name) { return apply_name_obfuscations_any(name); }
const char *apply_name_obfuscations(const char *name) { return apply_name_obfuscations_any(name); }
