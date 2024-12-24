cmake_minimum_required(VERSION 3.16)

project(UltimatePhobia LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../deps/detours Detours)
add_subdirectory(../deps/spdlog spdlog)

add_library(imgui STATIC
    ../deps/imgui/imgui.cpp ../deps/imgui/imgui_draw.cpp ../deps/imgui/imgui_tables.cpp ../deps/imgui/imgui_widgets.cpp
    ../deps/imgui/backends/imgui_impl_win32.cpp ../deps/imgui/backends/imgui_impl_dx11.cpp
)
target_link_libraries(imgui PUBLIC d3d11)
target_include_directories(imgui PUBLIC ../deps/imgui/ ../deps/imgui/backends/)

add_library(UltimatePhobia SHARED main.cpp
    detours_helpers.hpp
    global_state.hpp global_state.cpp
    getbaseaddr.hpp getbaseaddr.cpp
    misc_utils.hpp misc_utils.cpp
    application.hpp application.cpp
    gamedata.hpp gamedata.cpp
    game_hook.hpp game_hook.cpp
    imgui_man.hpp imgui_man.cpp)
target_link_libraries(UltimatePhobia PRIVATE Detours spdlog imgui)
target_include_directories(UltimatePhobia PRIVATE ../il2cpp_dump)
target_precompile_headers(UltimatePhobia PRIVATE ../il2cpp_dump/il2cpp.h)
