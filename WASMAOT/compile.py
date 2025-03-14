from sys import argv, exit
from os import mkdir, chdir
from subprocess import call

# Check arguments
if len(argv) != 5:
    print(f"Usage: {argv[0]} <WASM module> <extra export,...> <has _initialize function?> <compile for host platform?>")
    exit(-1)

# Get arguments
def str_to_bool(v):
    return v.lower() in ["yes", "true", "1"]
module = argv[1]
extra_exports = argv[2].split(',')
if len(extra_exports) == 1 and len(extra_exports[0]) == 0:
    extra_exports = []
has_initialize_func = str_to_bool(argv[3])
for_host_plat = str_to_bool(argv[4])

# Read template file to get main source
print("Filling source template...")
main_source = open("wasmaot.cpp-template").read()
extra_exports_str = ""
for export in extra_exports:
    extra_exports_str += f"EXPORT_FUNC({export})\n"
main_source = main_source.replace("ADD_extra_exports_here", extra_exports_str)
main_source = main_source.replace("ADD_has_initialize_func_here", "1" if has_initialize_func else "0")

# Write source file
open("wasmaot.cpp", "w").write(main_source)

# Execute wasm2c
print("Converting WASM to C...")
call(["wasm2c", "-n", "Module", "-o", "Module.c", "--enable-exceptions", module])

# Prepare build dir
print("Preparing build dir...")
call(["rm", "-rf", "build-aot"])
mkdir("build-aot")
chdir("build-aot")

# Run CMake and compiler
print("Compiling DLL...")
if not for_host_plat:
    call(["cmake", "..", "-GNinja", "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_C_COMPILER=clang-19", "-DCMAKE_CXX_COMPILER=clang++-19", "-DCMAKE_EXE_LINKER_FLAGS:STRING=-fuse-ld=lld", "-DCMAKE_CXX_FLAGS:STRING=--target=x86_64-windows-msvc", "-DCMAKE_C_FLAGS:STRING=--target=x86_64-windows-msvc", "-DCMAKE_SYSTEM_NAME:STRING=Windows"])
else:
    call(["cmake", "..", "-GNinja", "-DCMAKE_BUILD_TYPE=Release"])
call(["ninja"])

print("Module has been generated at build-aot/WASMAOT.{dll,so,dylib} if everything went well!")
