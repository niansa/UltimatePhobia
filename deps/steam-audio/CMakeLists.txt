cmake_minimum_required(VERSION 3.15)


add_library(phonon INTERFACE)
target_link_libraries(phonon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/phonon.lib)
target_include_directories(phonon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
