cmake_minimum_required(VERSION 3.31.6)

add_executable(demo main.c)
target_link_libraries(demo raycast SDL3::SDL3)
