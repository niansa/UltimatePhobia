from sys import argv
from sys import exit
import re
import json

if len(argv) < 3:
    print(f"Usage: {argv[0]} <script.json file> <WASM mode?>")
    exit(0)



wasm_mode = argv[2].lower() in ['true', '1', 't', 'y', 'yes', 'true', 'on']
if not wasm_mode:
    output_hpp = open("generated/il2cpp.hpp", "w")
    output_cpp = open("generated/il2cpp.cpp", "w")
else:
    output_hpp = open("generated/il2cpp_wasm.hpp", "w")
    output_cpp = open("generated/il2cpp_wasm.cpp", "w")

if not wasm_mode:
    method_pointer = "void *"
else:
    method_pointer = "::WASMInterface::MethodHandle "

def write_hpp(v):
    output_hpp.write(v)
def write_cpp(v):
    output_cpp.write(v)
def write_both(v):
    output_hpp.write(v)
    output_cpp.write(v)

write_hpp("#pragma once\n\n")
if not wasm_mode:
    write_cpp(f"#include \"il2cpp.hpp\"\n")
    write_cpp(f"#include \"global_state.hpp\"\n")
else:
    write_cpp(f"#include \"il2cpp_wasm.hpp\"\n")
    write_both(f"#include \"../wasm_interface.hpp\"\n")

write_both("""

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

if wasm_mode:
    def translate_type(type_name):
        type_name = type_name.strip()
        if type_name in ["void", "bool", "int32_t", "int64_t", "float", "double"]:
            return type_name
        elif type_name.count("*") == 1:
            return "::WASMInterface::ObjectHandle"
        else:
            print(f"Unsupported type: {type_name}!")
            return None
else:
    def translate_type(type_name):
        return type_name.strip()

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
            write_hpp(f"template<typename... T> {method_pointer}{name}_getPtr() = delete;\n")
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
    unsupported_type = False
    for arg in arguments.split(','):
        arg_type, arg_name = arg.rsplit(' ', 1)
        if len(arg_name) == 0:
            arg_name = "_"
        while arg_name in args_names:
            arg_name += "_"
        arg_type = translate_type(arg_type)
        if arg_type == None:
            unsupported_type = True
        args_names.append(arg_name)
        args_types.append(arg_type)
    return_type = translate_type(return_type)

    # Skip unsupported types
    if return_type == None or unsupported_type:
        continue

    # Filter reserved keywords
    for idx in range(len(args_names)):
        args_names[idx] = ensure_no_keyword(args_names[idx])

    # Get lists as strings
    arguments = []
    for idx in range(len(args_names)):
        arguments.append(args_types[idx]+" "+args_names[idx])
    arguments = ", ".join(arguments)
    args_names = ", ".join(args_names)
    args_types_nomi = ", ".join(args_types[:-1])
    args_types = ", ".join(args_types)

    # Ignore intptr_t types
    if "intptr_t" in args_types:
        continue

    # Ignore duplicate functions
    identifier = f"{full_name}<{args_types}>"
    if identifier in dedup:
        continue
    dedup[identifier] = None

    write_both(enter_namespace(namespace))

    # Generate pointer getter
    if method_name_duplicates[full_name]:
        write_both(f"template<> {method_pointer}{name}_getPtr<{args_types_nomi}>()")
    else:
        write_both(f"{method_pointer}{name}_getPtr()")
    write_hpp(";\n")
    if not wasm_mode:
        write_cpp(f" {{return reinterpret_cast<void *>({address}ul + g.base);}}\n")
    else:
        write_cpp(f" {{return ::WASMInterface::getMethodByIdentifier({json.dumps(raw_signature)});}}\n")

    # Generate function
    write_hpp(f"/**\n * Name: {json.dumps(raw_name)}\n * Signature: {json.dumps(raw_signature)}\n */\n")
    write_both(f"{return_type} {name}({arguments}")
    if not wasm_mode:
        write_hpp(" = nullptr);\n")
        write_cpp(f") {{return reinterpret_cast<{return_type} (*) ({args_types})>(reinterpret_cast<void *>(({address}ul + ::g.base)))({args_names});}}\n")
    else:
        write_hpp(" = ::WASMInterface::ObjectHandle::Null);\n")
        write_cpp(f") {{return ::Helpers::call<{json.dumps(raw_signature)}, {return_type}>({args_names});}}\n")

    write_both(leave_namespace(namespace))
    write_cpp("\n")

write_both("}\n")
output_hpp.close()
output_cpp.close()
