import itertools

template_top = """#include "../anycall.hpp"

#include <string>
#include <stdexcept>

namespace AnyCall {
uintptr_t call(const uintptr_t *args, void *fnc, std::string_view signature) {"""
template_bottom = """

    throw std::runtime_error("Unknown function signature: " + std::string(signature));
}
} // namespace AnyCall
"""

max_args = 6
types = {'v': "void", 'i': "uintptr_t", 'f': "float", 'd': "double"}

file = open("generated/anycall.cpp", "w")
file.write(template_top)

def generate_signatures():
    first_chars = []
    for char, _ in types.items():
        first_chars.append(char)
    other_chars = first_chars[1:]

    for length in range(1, 8):
        if length == 1:
            for c in first_chars:
                yield c
        else:
            for first in first_chars:
                for rest in itertools.product(other_chars, repeat=length-1):
                    yield first + ''.join(rest)

def generate_cast(char):
    return f"bit_cast<{types[char]}>"

def generate_arg(signature, index):
    return f"{generate_cast(signature[index])}(args[{index-1}])"

def generate_args(signature):
    fres = []
    for index in range(1, len(signature)):
        fres.append(generate_arg(signature, index))
    return ", ".join(fres)

def generate_arg_types(signature):
    fres = []
    for index in range(1, len(signature)):
        fres.append(types[signature[index]])
    return ", ".join(fres)

for signature in generate_signatures():
    file.write(f"\n    if (signature == \"{signature}\")\n        ")
    return_type = types[signature[0]]
    call = f"reinterpret_cast<{return_type} (*)({generate_arg_types(signature)})>(fnc)({generate_args(signature)})"
    if return_type != "void":
        file.write(f"return bit_cast<uintptr_t>({call});")
    else:
        file.write(f"{{{call}; return 0;}}")

file.write(template_bottom)
