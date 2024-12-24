cmake_minimum_required(VERSION 3.5)

project(Detours LANGUAGES CXX)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(Detours STATIC ${SRC_FILES})

# This file is included and not compiled on its own
set_property (
    SOURCE ${PROJECT_SOURCE_DIR}/src/uimports.cpp
    APPEND PROPERTY HEADER_FILE_ONLY true)

target_include_directories(Detours PUBLIC ${PROJECT_SOURCE_DIR}/src/)
