from sys import argv
from sys import exit
import itertools
import json


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

max_args = 5
types = {'v': "void", 'i': "uintptr_t", 'j': "uintptr_t", 'f': "float", 'd': "double"}


if len(argv) < 2:
    print(f"Usage: {argv[0]} <script.json file>")
    exit(1)

script_json = argv[1]


file = open("generated/anycall.cpp", "w")
file.write(template_top)

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


print("Loading script.json...")
script = json.load(open(script_json, "r"))

print("Collecting signatures...")
signatures = {}
for method in script["ScriptMethod"]:
    signatures[method["TypeSignature"]] = None

del script

print("Generating callers...")
for signature, _ in signatures.items():
    file.write(f"\n    if (signature == \"{signature}\")\n        ")
    return_type = types[signature[0]]
    call = f"reinterpret_cast<{return_type} (*)({generate_arg_types(signature)})>(fnc)({generate_args(signature)})"
    if return_type != "void":
        file.write(f"return bit_cast<uintptr_t>({call});")
    else:
        file.write(f"{{{call}; return 0;}}")

file.write(template_bottom)
