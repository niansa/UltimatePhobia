cmake_minimum_required(VERSION 3.15)


add_library(phonon INTERFACE)
if (WIN32)
    target_link_libraries(phonon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/phonon.lib)
else()
    target_link_libraries(phonon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libphonon.so)
endif()
target_include_directories(phonon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
