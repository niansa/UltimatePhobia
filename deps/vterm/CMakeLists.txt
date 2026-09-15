cmake_minimum_required(VERSION 3.20)

project(
    libvterm
    VERSION 0.3.3
    DESCRIPTION "An abstract C99 terminal emulator library"
    LANGUAGES C
)

include(GNUInstallDirs)

option(VTERM_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(VTERM_ENABLE_PROFILING "Compile and link with profiling support" OFF)

# ---------------------------------------------------------------------------
# Common compiler settings
# ---------------------------------------------------------------------------

function(vterm_configure_target target)
    set_target_properties(
        ${target}
        PROPERTIES
            C_STANDARD 99
            C_STANDARD_REQUIRED YES
            C_EXTENSIONS NO
    )

    if(VTERM_ENABLE_WARNINGS)
        if(MSVC)
            target_compile_options(${target} PRIVATE /W4)
        elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            target_compile_options(${target} PRIVATE -Wall -Wpedantic)
        endif()
    endif()

    # Preserve the original Makefile's Solaris feature-test macros.
    if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
        target_compile_definitions(
            ${target}
            PRIVATE
                __EXTENSIONS__
                _XPG6
                __XOPEN_OR_POSIX
        )
    endif()

    # CMake's Debug configuration already enables debug information.
    target_compile_definitions(
        ${target}
        PRIVATE
            $<$<CONFIG:Debug>:DEBUG>
    )

    if(VTERM_ENABLE_PROFILING)
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            target_compile_options(${target} PRIVATE -pg)
            target_link_options(${target} PRIVATE -pg)
        else()
            message(
                WARNING
                "VTERM_ENABLE_PROFILING was requested, but -pg is not known "
                "to be supported by ${CMAKE_C_COMPILER_ID}"
            )
        endif()
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Generated include files
#
# Release tarballs normally contain all of these already. In that case Perl
# is not searched for or required. Missing files are generated in the build
# tree instead of modifying the source tree.
# ---------------------------------------------------------------------------

file(
    GLOB VTERM_ENCODING_TABLES
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/encoding/*.tbl"
)
list(SORT VTERM_ENCODING_TABLES)

set(VTERM_INCLUDE_FRAGMENTS)
set(VTERM_GENERATED_OUTPUTS)
set(VTERM_GENERATION_REQUIRED FALSE)

foreach(table IN LISTS VTERM_ENCODING_TABLES)
    get_filename_component(table_name "${table}" NAME_WE)

    set(source_inc
        "${PROJECT_SOURCE_DIR}/src/encoding/${table_name}.inc"
    )
    set(generated_inc
        "${PROJECT_BINARY_DIR}/generated/src/encoding/${table_name}.inc"
    )

    if(EXISTS "${source_inc}")
        list(APPEND VTERM_INCLUDE_FRAGMENTS "${source_inc}")
    else()
        list(APPEND VTERM_INCLUDE_FRAGMENTS "${generated_inc}")
        list(APPEND VTERM_GENERATED_OUTPUTS "${generated_inc}")
        set(VTERM_GENERATION_REQUIRED TRUE)
    endif()
endforeach()

set(VTERM_SOURCE_FULLWIDTH_INC
    "${PROJECT_SOURCE_DIR}/src/fullwidth.inc"
)
set(VTERM_GENERATED_FULLWIDTH_INC
    "${PROJECT_BINARY_DIR}/generated/src/fullwidth.inc"
)

if(EXISTS "${VTERM_SOURCE_FULLWIDTH_INC}")
    list(APPEND
        VTERM_INCLUDE_FRAGMENTS
        "${VTERM_SOURCE_FULLWIDTH_INC}"
    )
else()
    list(APPEND
        VTERM_INCLUDE_FRAGMENTS
        "${VTERM_GENERATED_FULLWIDTH_INC}"
    )
    list(APPEND
        VTERM_GENERATED_OUTPUTS
        "${VTERM_GENERATED_FULLWIDTH_INC}"
    )
    set(VTERM_GENERATION_REQUIRED TRUE)
endif()

if(VTERM_GENERATION_REQUIRED)
    find_package(Perl REQUIRED)

    # CMake custom commands do not interpret shell redirection. This helper
    # captures a Perl script's stdout and writes it to the requested file in
    # a portable way.
    set(VTERM_RUN_PERL_SCRIPT
        "${PROJECT_BINARY_DIR}/cmake/RunPerlToFile.cmake"
    )
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/cmake")

    file(WRITE "${VTERM_RUN_PERL_SCRIPT}" [=[
if(NOT DEFINED PERL_EXECUTABLE
   OR NOT DEFINED SCRIPT
   OR NOT DEFINED OUTPUT_FILE
   OR NOT DEFINED WORKING_DIRECTORY)
    message(FATAL_ERROR "RunPerlToFile.cmake: missing required argument")
endif()

if(DEFINED INPUT_FILE AND NOT INPUT_FILE STREQUAL "")
    execute_process(
        COMMAND
            "${PERL_EXECUTABLE}" -CSD "${SCRIPT}" "${INPUT_FILE}"
        WORKING_DIRECTORY "${WORKING_DIRECTORY}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error_output
    )
else()
    execute_process(
        COMMAND
            "${PERL_EXECUTABLE}" -CSD "${SCRIPT}"
        WORKING_DIRECTORY "${WORKING_DIRECTORY}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error_output
    )
endif()

if(NOT result EQUAL 0)
    file(REMOVE "${OUTPUT_FILE}")
    message(
        FATAL_ERROR
        "Failed to generate ${OUTPUT_FILE}\n${error_output}"
    )
endif()

file(WRITE "${OUTPUT_FILE}" "${output}")
]=])

    set(VTERM_TBL2INC_SCRIPT
        "${PROJECT_SOURCE_DIR}/tbl2inc_c.pl"
    )
    set(VTERM_FIND_WIDE_CHARS_SCRIPT
        "${PROJECT_SOURCE_DIR}/find-wide-chars.pl"
    )

    foreach(table IN LISTS VTERM_ENCODING_TABLES)
        get_filename_component(table_name "${table}" NAME_WE)

        set(source_inc
            "${PROJECT_SOURCE_DIR}/src/encoding/${table_name}.inc"
        )

        if(NOT EXISTS "${source_inc}")
            if(NOT EXISTS "${VTERM_TBL2INC_SCRIPT}")
                message(
                    FATAL_ERROR
                    "${source_inc} is missing and tbl2inc_c.pl is unavailable"
                )
            endif()

            set(generated_inc
                "${PROJECT_BINARY_DIR}/generated/src/encoding/${table_name}.inc"
            )

            add_custom_command(
                OUTPUT "${generated_inc}"
                COMMAND
                    "${CMAKE_COMMAND}" -E make_directory
                    "${PROJECT_BINARY_DIR}/generated/src/encoding"
                COMMAND
                    "${CMAKE_COMMAND}"
                    "-DPERL_EXECUTABLE=${PERL_EXECUTABLE}"
                    "-DSCRIPT=${VTERM_TBL2INC_SCRIPT}"
                    "-DINPUT_FILE=${table}"
                    "-DOUTPUT_FILE=${generated_inc}"
                    "-DWORKING_DIRECTORY=${PROJECT_SOURCE_DIR}"
                    -P "${VTERM_RUN_PERL_SCRIPT}"
                DEPENDS
                    "${table}"
                    "${VTERM_TBL2INC_SCRIPT}"
                    "${VTERM_RUN_PERL_SCRIPT}"
                COMMENT "Generating src/encoding/${table_name}.inc"
                VERBATIM
            )
        endif()
    endforeach()

    if(NOT EXISTS "${VTERM_SOURCE_FULLWIDTH_INC}")
        if(NOT EXISTS "${VTERM_FIND_WIDE_CHARS_SCRIPT}")
            message(
                FATAL_ERROR
                "src/fullwidth.inc is missing and "
                "find-wide-chars.pl is unavailable"
            )
        endif()

        add_custom_command(
            OUTPUT "${VTERM_GENERATED_FULLWIDTH_INC}"
            COMMAND
                "${CMAKE_COMMAND}" -E make_directory
                "${PROJECT_BINARY_DIR}/generated/src"
            COMMAND
                "${CMAKE_COMMAND}"
                "-DPERL_EXECUTABLE=${PERL_EXECUTABLE}"
                "-DSCRIPT=${VTERM_FIND_WIDE_CHARS_SCRIPT}"
                "-DOUTPUT_FILE=${VTERM_GENERATED_FULLWIDTH_INC}"
                "-DWORKING_DIRECTORY=${PROJECT_SOURCE_DIR}"
                -P "${VTERM_RUN_PERL_SCRIPT}"
            DEPENDS
                "${VTERM_FIND_WIDE_CHARS_SCRIPT}"
                "${VTERM_RUN_PERL_SCRIPT}"
            COMMENT "Generating src/fullwidth.inc"
            VERBATIM
        )
    endif()
endif()

if(VTERM_GENERATED_OUTPUTS)
    set_source_files_properties(
        ${VTERM_GENERATED_OUTPUTS}
        PROPERTIES
            GENERATED TRUE
            HEADER_FILE_ONLY TRUE
    )
endif()

if(VTERM_INCLUDE_FRAGMENTS)
    set_source_files_properties(
        ${VTERM_INCLUDE_FRAGMENTS}
        PROPERTIES HEADER_FILE_ONLY TRUE
    )
endif()

# ---------------------------------------------------------------------------
# Library
# ---------------------------------------------------------------------------

file(
    GLOB VTERM_SOURCES
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.c"
)
file(
    GLOB VTERM_PUBLIC_HEADERS
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*.h"
)
file(
    GLOB VTERM_PRIVATE_HEADERS
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.h"
)

list(SORT VTERM_SOURCES)
list(SORT VTERM_PUBLIC_HEADERS)
list(SORT VTERM_PRIVATE_HEADERS)

add_library(vterm_library)

add_library(VTerm::vterm ALIAS vterm_library)

target_sources(
    vterm_library
    PRIVATE
        ${VTERM_SOURCES}
        ${VTERM_PRIVATE_HEADERS}
        ${VTERM_INCLUDE_FRAGMENTS}
    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS "${PROJECT_SOURCE_DIR}/include"
        FILES ${VTERM_PUBLIC_HEADERS}
)

target_include_directories(
    vterm_library
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        # Missing generated fragments are placed below this directory, so
        # includes such as "encoding/foo.inc" continue to work.
        "${PROJECT_BINARY_DIR}/generated/src"
)

vterm_configure_target(vterm_library)

set_target_properties(
    vterm_library
    PROPERTIES
        OUTPUT_NAME vterm
        EXPORT_NAME vterm

        # Equivalent to libtool -version-info 0:0:0.
        VERSION 0.0.0
        SOVERSION 0

        # Makes the shared-library build usable on Windows even when the
        # source headers do not use explicit dllexport declarations.
        WINDOWS_EXPORT_ALL_SYMBOLS YES
)

# ---------------------------------------------------------------------------
# pkg-config metadata
# ---------------------------------------------------------------------------

set(PREFIX  "${CMAKE_INSTALL_PREFIX}")
set(BINDIR  "${CMAKE_INSTALL_FULL_BINDIR}")
set(LIBDIR  "${CMAKE_INSTALL_FULL_LIBDIR}")
set(INCDIR  "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
set(MANDIR  "${CMAKE_INSTALL_FULL_MANDIR}")
set(MAN3DIR "${CMAKE_INSTALL_FULL_MANDIR}/man3")
set(VERSION "${PROJECT_VERSION}")

configure_file(
    "${PROJECT_SOURCE_DIR}/vterm.pc.in"
    "${PROJECT_BINARY_DIR}/vterm.pc"
    @ONLY
)

# ---------------------------------------------------------------------------
# Installation
# ---------------------------------------------------------------------------

install(
    TARGETS vterm_library
    EXPORT VTermTargets
    FILE_SET public_headers
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    RUNTIME
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    INCLUDES
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    FILES "${PROJECT_BINARY_DIR}/vterm.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

# ---------------------------------------------------------------------------
# CMake package metadata
# ---------------------------------------------------------------------------

include(CMakePackageConfigHelpers)

set(VTERM_CMAKE_INSTALL_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/VTerm"
)

set(VTERM_CONFIG_TEMPLATE
    "${PROJECT_BINARY_DIR}/cmake/VTermConfig.cmake.in"
)

file(WRITE "${VTERM_CONFIG_TEMPLATE}" [=[
@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/VTermTargets.cmake")

check_required_components(VTerm)
]=])

configure_package_config_file(
    "${VTERM_CONFIG_TEMPLATE}"
    "${PROJECT_BINARY_DIR}/VTermConfig.cmake"
    INSTALL_DESTINATION "${VTERM_CMAKE_INSTALL_DIR}"
)

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/VTermConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(
    EXPORT VTermTargets
    FILE VTermTargets.cmake
    NAMESPACE VTerm::
    DESTINATION "${VTERM_CMAKE_INSTALL_DIR}"
)

install(
    FILES
        "${PROJECT_BINARY_DIR}/VTermConfig.cmake"
        "${PROJECT_BINARY_DIR}/VTermConfigVersion.cmake"
    DESTINATION "${VTERM_CMAKE_INSTALL_DIR}"
)
