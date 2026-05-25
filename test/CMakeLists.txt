function(add_raycast_test name)
  set(test_src "${CMAKE_CURRENT_SOURCE_DIR}/test_${name}.c")
  set(runner_src "${CMAKE_CURRENT_BINARY_DIR}/test_${name}_runner.c")

  add_custom_command(
        OUTPUT ${runner_src}
        COMMAND ruby ${CMAKE_SOURCE_DIR}/external/Unity/auto/generate_test_runner.rb ${test_src} ${runner_src}
        DEPENDS ${test_src} ${CMAKE_SOURCE_DIR}/external/Unity/auto/generate_test_runner.rb
        COMMENT "Generating ${runner_src}"
    )

  # Define the test executable
  add_executable(${name}_tests ${test_src} ${runner_src})
  target_link_libraries(${name}_tests raycast_static Unity)

  # On MinGW, statically link the GCC and threading runtimes so the test
  # executable does not depend on libgcc_s_seh-1.dll / libwinpthread-1.dll
  # being present on PATH at test time.
  if(MINGW)
    target_link_options(${name}_tests PRIVATE -static)
  endif()

  # Register as a CTest test
  add_test(NAME ${name} COMMAND ${name}_tests)
endfunction()

add_raycast_test(raycast)
