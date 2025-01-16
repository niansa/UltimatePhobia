cmake_minimum_required(VERSION 3.16)

project(WASMTest LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../deps/crosslibc clibc)

add_executable(WASMTest main.cpp)
target_link_directories(WASMTest PUBLIC crosslibc)
target_precompile_headers(WASMTest PUBLIC <wasm32sdk.h>)
