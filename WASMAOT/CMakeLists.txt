cmake_minimum_required(VERSION 3.16)

project(WASMAOT LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

add_library(WASMAOT SHARED
  WASMAOT_global.hpp
  wasmaot.hpp wasmaot.cpp
  Module.h Module.c

  wasm-rt.h
  wasm-rt-exceptions.h wasm-rt-exceptions-impl.c
  wasm-rt-impl.h wasm-rt-impl.c
  wasm-rt-mem-impl.c
)
target_compile_definitions(WASMAOT PRIVATE WASMAOT_LIBRARY FFI_EXT FFI_USES_FTABLE FFI_FTABLE=*imports)
target_include_directories(WASMAOT PRIVATE ../UltimatePhobia/ffi)
