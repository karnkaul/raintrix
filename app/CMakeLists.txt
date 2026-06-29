add_executable(${package_name})

target_link_libraries(${package_name} PRIVATE
  ${package_name}-lib
  clap::clap
)

target_sources(${package_name} PRIVATE
  src/main.cpp
)

if(RAINTRIX_INSTALL)
  include(GNUInstallDirs)
  install(TARGETS ${package_name})
endif()
