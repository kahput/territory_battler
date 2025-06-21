cmake_minimum_required(VERSION 3.5...4.0)
project(example)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")

# Enable AddressSanitizer for all targets

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(ext)
add_subdirectory(src)
