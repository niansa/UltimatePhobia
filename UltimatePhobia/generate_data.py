from sys import argv
from sys import exit
import re
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
""")
write_cpp("""#include "il2cpp.hpp"
#include "global_state.hpp"


namespace Il2Cpp {
""")

methods = []
method_name_duplicates = {}

def ensure_no_keyword(name):
    if name in ["this", "xor", "or", "and", "not", "struct", "class", "namespace", "export", "extern", "default", "explicit", "nullptr", "delete", "new", "__callback", "bool", "char", "float", "double", "short", "int", "long", "small", "MethodInfo"]:
        return name+"_"
    return name

def clean_name(name):
    return ensure_no_keyword(re.sub(r"[^a-zA-Z0-9_]", '_', name))

def get_namespace_name(raw_name):
    namespace, name = raw_name.rsplit("$$", 1)
    namespace = namespace.split(".")
    return [clean_name(v) for v in namespace], clean_name(name)

def get_full_name(namespace, name):
    return ".".join(namespace)+"$$"+name

def is_blacklisted(namespace, name):
    if name in ["_", "_object_object_", "ZeroMemory"]:
        return True
    for v in namespace:
        if v in ["_", "___________", "____________object__object_"]:
            return True
    return False

def enter_namespace(namespace):
    fres = ""
    for v in namespace:
        fres = f"{fres}namespace {v} {{"
    return fres+"\n"

def leave_namespace(namespace):
    return "}"*len(namespace)+"\n"

print("Loading script.json...")
script = json.load(open(argv[1], "r"))
print("Collecting methods...")
for method in script["ScriptMethod"]:
    raw_signature = method["Signature"]
    raw_name = method["Name"]
    # Extract return type, name and arguments
    return_type, sig_name, arguments = raw_signature.split(' ', 2)
    if sig_name == "________________________":
        continue
    namespace, name = get_namespace_name(raw_name)
    if is_blacklisted(namespace, name):
        continue
    full_name = get_full_name(namespace, name)
    address = method["Address"]
    arguments = arguments[:-2][1:] + "Info_il2cpp"
    methods.append((return_type, arguments, address, namespace, name, raw_name, raw_signature))
    if full_name in method_name_duplicates:
        duplicate_count = method_name_duplicates[full_name] + 1
        method_name_duplicates[full_name] = duplicate_count
        if duplicate_count == 1:
            write_hpp(enter_namespace(namespace))
            write_hpp(f"template<typename... T> void *{name}_getPtr() = delete;\n")
            write_hpp(leave_namespace(namespace))
    else:
        method_name_duplicates[full_name] = 0

print("Generating functions...")
dedup = {}
for data in methods:
    return_type, arguments, address, namespace, name, raw_name, raw_signature = data
    full_name = get_full_name(namespace, name)
    args_names = []
    args_types = []
    for arg in arguments.split(','):
        arg_type, arg_name = arg.rsplit(' ', 1)
        if len(arg_name) == 0:
            arg_name = "_"
        while arg_name in args_names:
            arg_name += "_"
        args_names.append(arg_name)
        args_types.append(arg_type)

    # Filter reserved keywords
    for idx in range(len(args_names)):
        args_names[idx] = ensure_no_keyword(args_names[idx])

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
    identifier = f"{full_name}<{args_types_nomi}>"
    if identifier in dedup:
        continue
    dedup[identifier] = None

    write_both(enter_namespace(namespace))

    # Generate pointer getter
    if method_name_duplicates[full_name]:
        write_both(f"template<> void *{name}_getPtr<{args_types_nomi}>()")
    else:
        write_both(f"void *{name}_getPtr()")
    write_hpp(";\n")
    write_cpp(f" {{return reinterpret_cast<void *>({address}ul + g.base);}}\n")

    # Generate function
    write_hpp(f"/**\n * Name: {json.dumps(raw_name)}\n * Signature: {json.dumps(raw_signature)}\n */\n")
    write_both(f"{return_type} {name}({arguments}")
    write_hpp(" = nullptr);\n")
    write_cpp(f") {{return reinterpret_cast<{return_type} (*) ({args_types})>(reinterpret_cast<void *>(({address}ul + ::g.base)))({args_names});}}\n")

    write_both(leave_namespace(namespace))
    write_cpp("\n")

write_both("}\n")
output_hpp.close()
output_cpp.close()
