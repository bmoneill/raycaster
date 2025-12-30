cmake_minimum_required(VERSION 3.31.6)

# C standard
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_FILE_OFFSET_BITS=64")

# Warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-missing-field-initializers")

add_executable(untextured untextured.c util.c)
target_link_libraries(untextured raycast SDL3::SDL3)

add_executable(textured textured.c util.c)
target_link_libraries(textured raycast SDL3::SDL3)
