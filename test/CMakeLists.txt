cmake_minimum_required(VERSION 3.31.6)

add_executable(raycast_tests test_raycast.c)
target_link_libraries(raycast_tests raycast Unity)
add_test(raycast raycast_tests)
