cmake_minimum_required (VERSION 3.15)

project (implement-shell-noammirjani)

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)
set (CMAKE_C_EXTENSIONS OFF)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options (/W4 /permissive- /Zc:externConstexpr /Zc:inline)
endif ()

add_executable(implement-shell-noammirjani "" include/Shell.h src/Shell.cpp)

add_subdirectory (include)
add_subdirectory (src)
