from sys import argv
from sys import exit
import json

if len(argv) == 1:
    print(f"Usage: {argv[0]} <script.json file>")
    exit(0)



output_hpp = open("generated/il2cpp.hpp", "w")
output_cpp = open("generated/il2cpp.cpp", "w")

def write_hpp(v):
    output_hpp.write(v)
def write_cpp(v):
    output_cpp.write(v)
def write_both(v):
    output_hpp.write(v)
    output_cpp.write(v)

write_hpp("""#pragma once

#include "il2cpp.h"


namespace Il2Cpp {
namespace Methods {
""")
write_cpp("""#include "il2cpp.hpp"
#include "global_state.hpp"


namespace Il2Cpp {
namespace Methods {
""")

methods = []
method_name_duplicates = {}

script = json.load(open(argv[1], "r"))
for method in script["ScriptMethod"]:
    # Extract return type, name and arguments
    return_type, name, arguments = method['Signature'].split(' ', 2)
    address = method['Address']
    arguments = arguments[:-2][1:] + "Info_il2cpp"
    if name == "________________________":
        continue
    methods.append((return_type, arguments, address, name))
    if name in method_name_duplicates:
        duplicate_count = method_name_duplicates[name] + 1
        method_name_duplicates[name] = duplicate_count
        if duplicate_count == 1:
            write_hpp(f"template<typename... T> void *{name}_getPtr() = delete;\n")
    else:
        method_name_duplicates[name] = 0

dedup = {}
for data in methods:
    return_type, arguments, address, name = data
    args_names = []
    args_types = []
    for arg in arguments.split(','):
        arg_type, arg_name = arg.rsplit(' ', 1)
        while arg_name in args_names:
            arg_name += "_"
        args_names.append(arg_name)
        args_types.append(arg_type)

    # Filter reserved keywords
    for idx in range(len(args_names)):
        if args_names[idx] in ["or", "and", "not", "__callback", "small"]:
            args_names[idx] += "_"

    # Get lists as strings
    arguments = []
    for idx in range(len(args_names)):
        arguments.append(args_types[idx]+" "+args_names[idx])
    arguments = ','.join(arguments)
    args_names = ','.join(args_names)
    args_types_nomi = ','.join(args_types[:-1])
    args_types = ','.join(args_types)

    # Ignore intptr_t types
    if "intptr_t" in args_types:
        continue

    # Ignore duplicate functions
    identifier = f"{name}<{args_types_nomi}>"
    if identifier in dedup:
        continue
    dedup[identifier] = None

    # Generate pointer getter
    if method_name_duplicates[name]:
        write_both(f"template<> void *{name}_getPtr<{args_types_nomi}>()")
    else:
        write_both(f"void *{name}_getPtr()")
    write_hpp(";\n")
    write_cpp(f" {{return reinterpret_cast<void *>({address}ul + g.base);}}\n")

    # Generate function
    write_both(f"{return_type} {name}({arguments}")
    write_hpp(" = nullptr);\n")
    write_cpp(f") {{return reinterpret_cast<{return_type} (*) ({args_types})>(reinterpret_cast<void *>(({address}ul + ::g.base)))({args_names});}}\n\n")

write_both("}\n}\n")
output_hpp.close()
output_cpp.close()



output = open("generated/game_data.inc", "w")
output.write("""
#include <string_view>
#include <vector>


static std::vector<GameData::Method> methods = {
""")

for method in script["ScriptMethod"]:
    output.write(f"    {{(void*){method['Address']}, {json.dumps(method['Name'])}, {json.dumps(method['Signature'])}, {json.dumps(method['TypeSignature'])}}},\n");

output.write("};")
output.close()
