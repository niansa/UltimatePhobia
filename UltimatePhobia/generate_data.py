from sys import argv
from sys import exit
import re
import json

if len(argv) < 4:
    print(f"Usage: {argv[0]} <script.json file> <output dir> <FFI mode?> [filter list]")
    exit(1)


script_json = argv[1]
output_dir = argv[2]
ffi_mode = argv[3].lower() in ['true', '1', 't', 'y', 'yes', 'true', 'on']
filter_list = None
try:
    filter_list = json.load(open(argv[4], "r"))
except IndexError:
    pass

if not ffi_mode:
    output_hpp = open(output_dir + "/il2cpp.hpp", "w")
    output_cpp = open(output_dir + "/il2cpp.cpp", "w")
else:
    output_hpp = open(output_dir + "/il2cpp_ffi.hpp", "w")
    output_cpp = open(output_dir + "/il2cpp_ffi.cpp", "w")

output_prof = open(output_dir + "/profiling.cpp", "w")

if not ffi_mode:
    method_pointer = "void *"
else:
    method_pointer = "::FFIInterface::MethodHandle "

def write_hpp(v):
    output_hpp.write(v)

def write_cpp(v):
    output_cpp.write(v)

def write_both(v):
    output_hpp.write(v)
    output_cpp.write(v)

def write_prof(v):
    output_prof.write(v)

write_hpp("#pragma once\n\n")
if not ffi_mode:
    write_cpp(f"#include \"il2cpp.hpp\"\n")
    write_cpp(f"#include \"global_state.hpp\"\n")
else:
    write_cpp(f"#include \"base.hpp\"\n")
    write_both(f"#include \"ffi_interface.hpp\"\n")

write_both("""

namespace Il2Cpp {
""")

methods = []
written_methods = []
method_name_duplicates = {}

def ensure_no_keyword(name):
    if name in ["this", "xor", "or", "and", "not", "struct", "class", "template", "namespace", "export", "extern", "default", "explicit", "nullptr", "delete", "new", "__callback", "bool", "char", "float", "double", "short", "int", "long", "small", "MethodInfo"]:
        return name + "_"
    return name

def clean_name(name):
    return ensure_no_keyword(re.sub(r"[^a-zA-Z0-9_]", '_', name))

def get_namespace_name(raw_name):
    namespace, name = raw_name.rsplit("$$", 1)
    namespace = namespace.split(".")
    return [clean_name(v) for v in namespace], clean_name(name)

def get_full_name(namespace, name):
    return ".".join(namespace) + "$$" + name

def is_blacklisted(namespace, name):
    if name in ["_", "_object_object_", "ZeroMemory"] or "___________" in name:
        return True
    if "StackTraceUtility" in namespace:
        return True
    if "ExtractStringFromException" in name:
        return True
    for v in namespace:
        if v in ["_", "___________", "____________object__object_"]:
            return True
    return False

def enter_namespace(namespace):
    fres = ""
    for v in namespace:
        fres = f"{fres}namespace {v} {{"
    return fres + "\n"

def leave_namespace(namespace):
    return "}" * len(namespace) + "\n"

def make_profile_ident(namespace, name, address, idx):
    return clean_name("hook_" + "_".join(namespace + [name, str(address), str(idx)]))

def get_qualified_cpp_name(namespace, name):
    if len(namespace) == 0:
        return f"::Il2Cpp::{name}"
    return "::Il2Cpp::" + "::".join(namespace + [name])

if ffi_mode:
    def translate_type(type_name):
        type_name = type_name.strip()
        if type_name in ["void", "bool", "int32_t", "int64_t", "float", "double"]:
            return type_name
        elif type_name.count("*") > 0:
            return "::FFIInterface::ObjectHandle"
        else:
            print(f"Unsupported type: {type_name}!")
            return None
else:
    def translate_type(type_name):
        return type_name.strip()

print("Loading script.json...")
script = json.load(open(script_json, "r"))
print("Collecting methods...")
for method in script["ScriptMethod"]:
    raw_signature = method["Signature"]
    raw_name = method["Name"]

    # Skip names not in filter list
    if filter_list != None and not any(raw_name.startswith(v) for v in filter_list):
        continue

    # Extract return type, name and arguments
    return_type, sig_name, arguments = raw_signature.split(' ', 2)
    if sig_name == "________________________":
        continue

    namespace, name = get_namespace_name(raw_name)
    if is_blacklisted(namespace, name):
        continue

    full_name = get_full_name(namespace, name)
    if "Address" not in method:
        continue

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

    arg_names_list = []
    arg_types_list = []
    unsupported_type = False

    for arg in arguments.split(','):
        arg_type, arg_name = arg.rsplit(' ', 1)
        if len(arg_name) == 0:
            arg_name = "_"
        while arg_name in arg_names_list:
            arg_name += "_"

        arg_type = translate_type(arg_type)
        if arg_type == None:
            unsupported_type = True

        arg_names_list.append(arg_name)
        arg_types_list.append(arg_type)

    return_type = translate_type(return_type)

    # Skip unsupported types
    if return_type == None or unsupported_type:
        continue

    # Filter reserved keywords
    for idx in range(len(arg_names_list)):
        arg_names_list[idx] = ensure_no_keyword(arg_names_list[idx])

    # Get lists as strings
    arguments_decl = []
    for idx in range(len(arg_names_list)):
        arguments_decl.append(arg_types_list[idx] + " " + arg_names_list[idx])
    arguments_decl = ", ".join(arguments_decl)

    args_names_call = ", ".join(arg_names_list)
    args_types_nomi = ", ".join(arg_types_list[:-1])
    args_types_str = ", ".join(arg_types_list)

    # Ignore intptr_t types
    if "intptr_t" in args_types_str:
        continue

    # Ignore duplicate functions
    identifier = f"{full_name}<{args_types_nomi}>"
    if identifier in dedup:
        continue
    dedup[identifier] = None

    # Record only methods that are actually emitted
    written_methods.append({
        "return_type": return_type,
        "address": address,
        "namespace": namespace,
        "name": name,
        "raw_name": raw_name,
        "raw_signature": raw_signature,
        "arguments_decl": arguments_decl,
        "args_names_call": args_names_call,
        "args_types_str": args_types_str,
    })

    write_both(enter_namespace(namespace))

    # Generate pointer getter
    if method_name_duplicates[full_name]:
        write_both(f"template<> {method_pointer}{name}_getPtr<{args_types_nomi}>()")
    else:
        write_both(f"{method_pointer}{name}_getPtr()")
    write_hpp(";\n")
    if not ffi_mode:
        write_cpp(f" {{return reinterpret_cast<void *>({address}ul + g.base);}}\n")
    else:
        write_cpp(f" {{return FFI_USE_FTABLE getMethodByIdentifier({json.dumps(raw_signature)});}}\n")

    # Generate function
    write_hpp(f"/**\n * Name: {json.dumps(raw_name)}\n * Signature: {json.dumps(raw_signature)}\n */\n")
    write_both(f"{return_type} {name}({arguments_decl}")
    if not ffi_mode:
        write_hpp(" = nullptr);\n")
        write_cpp(f") {{return reinterpret_cast<{return_type} (*) ({args_types_str})>(reinterpret_cast<void *>(({address}ul + ::g.base)))({args_names_call});}}\n")
    else:
        write_hpp(" = ::FFIInterface::ObjectHandle::Null);\n")
        write_cpp(f") {{return ::FFIInterface::Helpers::call<{json.dumps(raw_signature)}, {return_type}>({args_names_call});}}\n")

    write_both(leave_namespace(namespace))
    write_cpp("\n")

print("Generating profiling.cpp...")

if ffi_mode:
    write_prof("// profiling.cpp: profiling hooks are only generated in non-FFI mode.\n")
    write_prof("#include <cstddef>\n\n")
    write_prof("namespace Il2Cpp { namespace Profiling {\n")
    write_prof("void InstallHooks() {}\n")
    write_prof("std::size_t HookCount() { return 0; }\n")
    write_prof("} }\n")
else:
    address_counts = {}
    for m in written_methods:
        address_counts[m["address"]] = address_counts.get(m["address"], 0) + 1

    profiled_methods = []
    for m in written_methods:
        # Skip if the address is not unique
        if address_counts[m["address"]] != 1:
            continue

        # Explicitly forbid profiling UnityEngine and System namespaces
        if len(m["namespace"]) > 0:
            root_ns = m["namespace"][0]
            if root_ns in ["UnityEngine", "System"]:
                continue

        profiled_methods.append(m)

    write_prof("#include \"il2cpp.hpp\"\n")
    write_prof("#include \"global_state.hpp\"\n")
    write_prof("#include \"game_hook.hpp\"\n")
    write_prof("#include <tracy/Tracy.hpp>\n")
    write_prof("#include <cstddef>\n")
    write_prof("#include <cstdlib>\n\n")

    write_prof(f"// Emitted methods: {len(written_methods)}\n")
    write_prof(f"// Hooked methods with unique addresses: {len(profiled_methods)}\n")
    write_prof(f"// Skipped due to non-unique address: {len(written_methods) - len(profiled_methods)}\n\n")

    write_prof("namespace Il2Cpp {\n\n")

    for idx, m in enumerate(profiled_methods):
        hook_ident = make_profile_ident(m["namespace"], m["name"], m["address"], idx)
        hook_state_ident = hook_ident + "_state"
        fn_type = f"{m['return_type']}({m['args_types_str']})"

        write_prof(enter_namespace(m["namespace"]))
        write_prof(f"static GameHook *{hook_state_ident} = nullptr;\n")
        write_prof(f"static {m['return_type']} {hook_ident}({m['arguments_decl']}) {{\n")
        write_prof(f"    GameHookRelease release(*{hook_state_ident});\n")
        write_prof(f"    if (!release.isValid()) std::abort();\n")
        write_prof(f"    ZoneScopedN({json.dumps(m['raw_name'])});\n")
        write_prof(f"    auto original_fnc_ = {hook_state_ident}->getFunction<{fn_type}>();\n")
        if m["return_type"] == "void":
            write_prof(f"    original_fnc_({m['args_names_call']});\n")
        else:
            write_prof(f"    return original_fnc_({m['args_names_call']});\n")
        write_prof("}\n")
        write_prof(leave_namespace(m["namespace"]))
        write_prof("\n")

    write_prof("namespace Profiling {\n\n")
    write_prof("static bool g_installed = false;\n\n")
    write_prof(f"std::size_t HookCount() {{ return {len(profiled_methods)}; }}\n\n")
    write_prof("void InstallHooks() {\n")
    write_prof("    if (g_installed) return;\n")
    write_prof("    g_installed = true;\n")
    for idx, m in enumerate(profiled_methods):
        hook_ident = make_profile_ident(m["namespace"], m["name"], m["address"], idx)
        hook_state_ident = hook_ident + "_state"
        hook_fn_qname = get_qualified_cpp_name(m["namespace"], hook_ident)
        hook_state_qname = get_qualified_cpp_name(m["namespace"], hook_state_ident)
        write_prof(
            f"    {hook_state_qname} = GameHook::safeCreatePtr("
            f"reinterpret_cast<void *>(({m['address']}ul + ::g.base)), "
            f"reinterpret_cast<void *>(&{hook_fn_qname}), "
            f"false"
            f");\n"
        )
    write_prof("}\n\n")
    write_prof("} // namespace Profiling\n")
    write_prof("} // namespace Il2Cpp\n")

write_both("}\n")
output_hpp.close()
output_cpp.close()
output_prof.close()
