file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src.zip" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")

message(STATUS "[le2d]")
add_subdirectory(src/le2d)

message(STATUS "[clap]")
add_subdirectory(src/clap)
