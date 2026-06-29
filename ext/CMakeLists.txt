file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src.zip" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")

message(STATUS "[le2d]")
set(SKIP_INSTALL_ALL ON) # disable freetype install.
add_subdirectory(src/le2d)

message(STATUS "[clap]")
add_subdirectory(src/clap)
