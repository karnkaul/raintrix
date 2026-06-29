cmake_minimum_required(VERSION 4.3)

set(package_name raintrix)
set(build_version 0.1.1)
project(${package_name} VERSION ${build_version})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

add_subdirectory(ext)
add_subdirectory(lib)
add_subdirectory(app)
