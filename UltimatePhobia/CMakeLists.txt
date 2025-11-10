cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

project(UltimatePhobia LANGUAGES RC CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

include(CheckLinkerFlag)

option(ULTIMATEPHOBIA_CHEATS "Build cheats module" ON)
if (ULTIMATEPHOBIA_CHEATS)
    option(ULTIMATEPHOBIA_EXTRA_CHEATS "Build extra cheats module" OFF)
endif()

function(extract_first_line input_string output_var)
    # Replace newlines with semicolons to create a list
    string(REPLACE "\n" ";" lines_list "${input_string}")

    # Get the first element (first line)
    list(GET lines_list 0 first_line)

    set(${output_var} "${first_line}" PARENT_SCOPE)
endfunction()

function(get_compiler_info)
    # Get compiler ID and version
    set(COMPILER_ID ${CMAKE_CXX_COMPILER_ID})
    set(COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})

    # Try to get full version string
    if(COMPILER_ID STREQUAL "GNU")
        execute_process(
            COMMAND ${CMAKE_CXX_COMPILER} --version
            OUTPUT_VARIABLE VERSION_STRING
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    elseif(COMPILER_ID STREQUAL "Clang")
        execute_process(
            COMMAND ${CMAKE_CXX_COMPILER} --version
            OUTPUT_VARIABLE VERSION_STRING
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    elseif(COMPILER_ID STREQUAL "MSVC")
        # MSVC version is already in CMAKE_CXX_COMPILER_VERSION
        set(VERSION_STRING "Microsoft Visual C++ ${COMPILER_VERSION}")
    else()
        set(VERSION_STRING "Unknown compiler: ${COMPILER_ID} ${COMPILER_VERSION}")
    endif()

    extract_first_line("${VERSION_STRING}" VERSION_STRING)

    message("Compiler: ${COMPILER_ID}")
    message("Version: ${COMPILER_VERSION}")
    message("Full version string: ${VERSION_STRING}")

    set(COMPILER_INFO "${VERSION_STRING}" PARENT_SCOPE)
endfunction()
get_compiler_info()

if (MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections -fno-threadsafe-statics")
endif()

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_VIDEO ON)
set(SDL_RENDER ON)
set(SDL_AUDIO OFF)
set(SDL_GPU OFF)
set(SDL_CAMERA OFF)
set(SDL_JOYSTICK OFF)
set(SDL_HAPTIC OFF)
set(SDL_HIDAPI OFF)
set(SDL_POWER OFF)
set(SDL_SENSOR OFF)
set(SDL_DIALOG OFF)
set(SDL_TESTS OFF)

add_subdirectory(../deps/commoncpp commoncpp)
add_subdirectory(../deps/spdlog spdlog)
add_subdirectory(../deps/wasm3/source wasm3)
add_subdirectory(../deps/wasm3/platforms/cpp/wasm3_cpp wasm3_cpp)
add_subdirectory(../deps/SDL sdl3)
add_subdirectory(../deps/elfld elfld)

add_library(imgui STATIC
    ../deps/imgui/imgui.cpp ../deps/imgui/imgui_draw.cpp ../deps/imgui/imgui_tables.cpp ../deps/imgui/imgui_widgets.cpp
    ../deps/imgui/backends/imgui_impl_sdl3.cpp ../deps/imgui/backends/imgui_impl_sdlrenderer3.cpp
)
target_link_libraries(imgui PUBLIC SDL3::SDL3-static)
target_include_directories(imgui PUBLIC ../deps/imgui/ ../deps/imgui/backends/)

add_executable(UltimatePhobia WIN32 main.cpp icon.rc
    global_state.hpp global_state.cpp
    getbaseaddr.hpp getbaseaddr.cpp
    misc_utils.hpp misc_utils.cpp
    application.hpp application.cpp
    il2cpp_dynamic.hpp il2cpp_dynamic.cpp
    game_hook.hpp game_hook.cpp
    imgui_man.hpp imgui_man.cpp
    game_enums.hpp
    mods/base.hpp
    mods/global_instance_manager.hpp mods/global_instance_manager.cpp
    mods/cheats.hpp mods/cheats.cpp
    mods/improvements.hpp mods/improvements.cpp
    mods/tracer.hpp mods/tracer.cpp
    mods/photon_settings.hpp mods/photon_settings.cpp
    il2cpp_cppinterop.hpp il2cpp_cppinterop.cpp
    mods/save_file_manager.hpp mods/save_file_manager.cpp
    bindings/unityengine.hpp bindings/unityengine.cpp
    mods/goldberg_emu_manager.hpp mods/goldberg_emu_manager.cpp
    disable_antimod.hpp disable_antimod.cpp
    crash_handler.hpp crash_handler.cpp
    disable_splashscreen.hpp disable_splashscreen.cpp
    mods/player_manager.hpp mods/player_manager.cpp
    mods/fixes.hpp mods/fixes.cpp
    generated/il2cpp.hpp generated/il2cpp.cpp
    safe_path.hpp safe_path.cpp
    ffi_loader.hpp ffi_loader.cpp
    ffi_interface.hpp ffi_interface.cpp
    ffi.hpp
    wasm_ffi.hpp wasm_ffi.cpp
    dll_ffi.hpp dll_ffi.cpp
    elf_ffi.hpp elf_ffi.cpp
    sock_ffi.hpp sock_ffi.cpp
    configure_persistentdatapath.hpp configure_persistentdatapath.cpp
    il2cpp_api.hpp il2cpp_api.cpp
    anycall.hpp
    generated/anycall.cpp
    il2cpp_api_cpp.hpp
    mods/explorer.hpp mods/explorer.cpp
    deobfuscations.hpp deobfuscations.cpp
    il2cpp_api_cpp.cpp
)
target_link_libraries(UltimatePhobia PRIVATE dbghelp commoncpp spdlog imgui m3 wasm3_cpp ws2_32 elfldpp)
target_include_directories(UltimatePhobia PRIVATE . ../il2cpp_dump)
target_precompile_headers(UltimatePhobia PRIVATE ../il2cpp_dump/il2cpp.h generated/il2cpp.hpp)
#target_precompile_headers(UltimatePhobia PRIVATE ../il2cpp_dump/il2cpp.h)
target_compile_definitions(UltimatePhobia PUBLIC _CRT_SECURE_NO_WARNINGS COMPILER_INFO="${COMPILER_INFO}")
if (ULTIMATEPHOBIA_CHEATS)
    target_compile_definitions(UltimatePhobia PRIVATE MOD_ENABLE_CHEATS)
    if (ULTIMATEPHOBIA_EXTRA_CHEATS)
        target_compile_definitions(UltimatePhobia PRIVATE MOD_ENABLE_CHEATS_EXTRA)
    endif()
endif()
check_linker_flag(CXX -Wl,--subsystem,windows LINKER_CAN_SELECT_SUBSYSTEM)
if (LINKER_CAN_SELECT_SUBSYSTEM)
    target_link_options(UltimatePhobia PRIVATE -Wl,--subsystem,windows)
endif()
if (MINGW)
    target_link_options(UltimatePhobia PRIVATE -Wl,--gc-sections -static-libstdc++)
    target_link_libraries(UltimatePhobia PRIVATE libunwind.a)
endif()

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/generated/il2cpp.hpp ${CMAKE_CURRENT_SOURCE_DIR}/generated/il2cpp.cpp
                   COMMAND python3 generate_data.py ../il2cpp_dump/script.json generated/ false generation_filter.json
                   DEPENDS generate_data.py ../il2cpp_dump/script.json generation_filter.json
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
