cmake_minimum_required(VERSION 3.16)
project(Slugger VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create library
add_library(Slugger
    src/slugger.cpp
    include/Slugger/slugger.hpp
)

target_include_directories(Slugger
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
)

# Link dependencies
target_link_libraries(Slugger PUBLIC
    DirectXMath
    d3d11
    d3dcompiler
    dxgi
    dwmapi
    dcomp
)

# Shader embedding
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_DIR}")

set(EMBED_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/embed_shader.cmake")
file(WRITE "${EMBED_SCRIPT}" "
file(READ \"\${INPUT_FILE}\" CONTENT HEX)
string(REGEX MATCHALL \"..\" HEX_CHARS \"\${CONTENT}\")
set(HEX_ARRAY \"\")
foreach(CHAR \${HEX_CHARS})
    set(HEX_ARRAY \"\${HEX_ARRAY}0x\${CHAR}, \")
endforeach()
# Null-terminate the string so D3DCompile can read it safely
file(WRITE \"\${OUTPUT_FILE}\" \"#pragma once\\nconstexpr unsigned char \${VAR_NAME}[] = { \${HEX_ARRAY} 0x00 };\\n\")
")

function(embed_shader SHADER_FILE VAR_NAME OUTPUT_HEADER)
    add_custom_command(
        OUTPUT "${OUTPUT_HEADER}"
        COMMAND ${CMAKE_COMMAND} 
            -D INPUT_FILE="${SHADER_FILE}" 
            -D OUTPUT_FILE="${OUTPUT_HEADER}" 
            -D VAR_NAME="${VAR_NAME}"
            -P "${EMBED_SCRIPT}"
        DEPENDS "${SHADER_FILE}"
        COMMENT "Embedding ${SHADER_FILE} into ${OUTPUT_HEADER}..."
    )
endfunction()

embed_shader("${SHADER_DIR}/SlugVertexShader.hlsl" "g_SlugVertexShader" "${GENERATED_DIR}/SlugVertexShader.h")
embed_shader("${SHADER_DIR}/SlugPixelShader.hlsl" "g_SlugPixelShader" "${GENERATED_DIR}/SlugPixelShader.h")

# Ensure library depends on generated headers
target_sources(Slugger PRIVATE 
    "${GENERATED_DIR}/SlugVertexShader.h"
    "${GENERATED_DIR}/SlugPixelShader.h"
)

# Create example application
add_executable(SluggerExample WIN32 src/example.cpp)
target_link_libraries(SluggerExample Slugger)
target_compile_definitions(SluggerExample PUBLIC
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
    NTDDI_VERSION=0x0A000000
)
if (MINGW)
    target_link_options(SluggerExample PRIVATE -static)
endif()
