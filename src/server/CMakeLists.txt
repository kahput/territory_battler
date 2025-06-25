file(GLOB_RECURSE SOURCES "./*.c" "./*.h" "${PROJECT_SOURCE_DIR}/src/common/*.c" "${PROJECT_SOURCE_DIR}/src/common/*.h")
add_executable(server ${SOURCES})
target_include_directories(server PRIVATE ${PROJECT_SOURCE_DIR}/src)
