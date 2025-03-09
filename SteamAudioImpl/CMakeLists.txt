cmake_minimum_required(VERSION 3.16)

project(SteamAudioImpl LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

add_subdirectory(../deps/steam-audio steam-audio)

add_library(SteamAudioImpl SHARED
  SteamAudioImpl_global.hpp
  SteamAudioImpl.hpp SteamAudioImpl.cpp

  miniaudio.c
)
target_link_libraries(SteamAudioImpl PRIVATE phonon)
target_compile_definitions(SteamAudioImpl PRIVATE STEAMAUDIOIMPL_LIBRARY FFI_EXT FFI_USES_FTABLE)
target_include_directories(SteamAudioImpl PRIVATE ../UltimatePhobia/ffi)
target_compile_options(SteamAudioImpl PUBLIC -fno-exceptions)
