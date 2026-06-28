project(${package_name}-lib VERSION ${build_version})

add_library(${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME} PRIVATE
  le2d::le2d
)

set(version_header "${CMAKE_CURRENT_BINARY_DIR}/include/raintrix/build_version.hpp")
configure_file(src/build_version.hpp.in "${version_header}" @ONLY)

target_include_directories(${PROJECT_NAME} PRIVATE
  src
)

target_sources(${PROJECT_NAME} PUBLIC FILE_SET HEADERS
  BASE_DIRS
  include
  "${CMAKE_CURRENT_BINARY_DIR}/include"

  FILES
  include/raintrix/raintrix.hpp
  "${version_header}"
)

target_sources(${PROJECT_NAME} PRIVATE
  src/raintrix.cpp
)
