#include "deobfuscations.hpp"

#include <string_view>

template <typename StrT> StrT apply_name_deobfuscations_any(StrT name) {
    if (std::string_view(name) == "\u0a68\u0a69\u0a6f\u0a67\u0a68\u0a67\u0a70\u0a67\u0a69\u0a73\u0a67")
        name = "Use";
    return name;
}

std::string_view apply_name_deobfuscations(std::string_view name) { return apply_name_deobfuscations_any(name); }
const char *apply_name_deobfuscations(const char *name) { return apply_name_deobfuscations_any(name); }
