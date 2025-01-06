cmake_minimum_required(VERSION 3.16)

project(UltimatePhobiaLauncher LANGUAGES RC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Phasmophobia main.cpp icon.rc)
