cmake_minimum_required(VERSION 3.16)

project(UltimatePhobia LANGUAGES RC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

option(ULTIMATEPHOBIA_CHEATS "Build cheats module" ON)
option(ULTIMATEPHOBIA_TRACER "Build tracer module" OFF)

add_subdirectory(../deps/detours Detours)
add_subdirectory(../deps/spdlog spdlog)

add_library(imgui STATIC
    ../deps/imgui/imgui.cpp ../deps/imgui/imgui_draw.cpp ../deps/imgui/imgui_tables.cpp ../deps/imgui/imgui_widgets.cpp
    ../deps/imgui/backends/imgui_impl_win32.cpp ../deps/imgui/backends/imgui_impl_dx11.cpp
)
target_link_libraries(imgui PUBLIC d3d11)
target_include_directories(imgui PUBLIC ../deps/imgui/ ../deps/imgui/backends/)

add_executable(UltimatePhobia WIN32 main.cpp icon.rc
    detours_helpers.hpp
    global_state.hpp global_state.cpp
    getbaseaddr.hpp getbaseaddr.cpp
    misc_utils.hpp misc_utils.cpp
    application.hpp application.cpp
    gamedata.hpp gamedata.cpp
    game_hook.hpp game_hook.cpp
    imgui_man.hpp imgui_man.cpp
    game_enums.hpp
    mods/base.hpp
    mods/global_instance_manager.hpp mods/global_instance_manager.cpp
    mods/cheats.hpp mods/cheats.cpp
    mods/tracer.hpp mods/tracer.cpp
    mods/photon_settings.hpp mods/photon_settings.cpp
    game_types.hpp game_types.cpp
    mods/save_file_manager.hpp mods/save_file_manager.cpp
    bindings/unityengine.hpp bindings/unityengine.cpp
    bindings/phasmophobia.hpp bindings/phasmophobia.cpp
    mods/goldberg_emu_manager.hpp mods/goldberg_emu_manager.cpp
    disable_antimod.hpp disable_antimod.cpp
    crash_handler.hpp crash_handler.cpp
    disable_splashscreen.hpp disable_splashscreen.cpp
    mods/player_manager.hpp mods/player_manager.cpp
    mods/fixes.hpp mods/fixes.cpp
    generated/il2cpp.hpp generated/il2cpp.cpp
)
target_link_libraries(UltimatePhobia PRIVATE dbghelp Detours spdlog imgui)
target_include_directories(UltimatePhobia PRIVATE . ../il2cpp_dump)
target_precompile_headers(UltimatePhobia PRIVATE ../il2cpp_dump/il2cpp.h)
target_compile_definitions(UltimatePhobia PUBLIC _CRT_SECURE_NO_WARNINGS)
if (ULTIMATEPHOBIA_CHEATS)
    target_compile_definitions(UltimatePhobia PRIVATE MOD_ENABLE_CHEATS)
endif()
if (ULTIMATEPHOBIA_TRACER)
    target_compile_definitions(UltimatePhobia PRIVATE MOD_ENABLE_TRACER ENABLE_OLD_GAMEDATA)
endif()
