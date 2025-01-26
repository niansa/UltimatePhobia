cmake_minimum_required(VERSION 3.16)

project(WASMTest LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

build_wasm32crt()

add_subdirectory(../../deps/crosslibc clibc)

add_executable(WASMTest main.cpp user_impl.cpp)
target_include_directories(WASMTest PUBLIC ../../UltimatePhobia/ffi)
target_link_libraries(WASMTest PUBLIC wasm32crt crosslibc)
target_precompile_headers(WASMTest PUBLIC <wasm32sdk.h>)
