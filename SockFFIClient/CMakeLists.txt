cmake_minimum_required(VERSION 3.16)

project(SockFFIClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

add_executable(SockFFIClient
    main.cpp
    socket.cpp
    utils.hpp
    dlhandle.hpp
)
target_include_directories(SockFFIClient PRIVATE ../UltimatePhobia/ffi)

add_library(SockFFIClientTest SHARED test.cpp)
target_include_directories(SockFFIClientTest PRIVATE ../UltimatePhobia/ffi)
target_compile_definitions(SockFFIClientTest PRIVATE SOCKFFICLIENTTEST_LIBRARY FFI_EXT FFI_USES_FTABLE FFI_FTABLE=*imports FFI=FFI_USE_FTABLE)

include(GNUInstallDirs)
install(TARGETS SockFFIClient
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
