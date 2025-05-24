file(GLOB_RECURSE SOURCES "./*.c" "./*.h" "${PROJECT_SOURCE_DIR}/ext/stb_image/*.c" "${PROJECT_SOURCE_DIR}/ext/stb_image/*.h")

find_package(raylib)

add_executable(client ${SOURCES})
target_link_libraries( client
    PUBLIC
        raylib
        glad_gl_core_45
        glfw
    )
