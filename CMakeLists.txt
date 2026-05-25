cmake_minimum_required(VERSION 3.31.6)
project("libraycast" C)
include(CTest)

option(TEST "Build tests" OFF)

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Version
add_compile_definitions(RAYCAST_VERSION="${GIT_COMMIT_HASH}")


# SDL3
include(FetchContent)
function(include_dependency libName gitURL gitTag)
  FetchContent_Declare(${libName}
            GIT_REPOSITORY ${gitURL}
            GIT_TAG        ${gitTag}
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   TRUE
    )

  FetchContent_MakeAvailable(${libName})
endfunction()
find_package(SDL3 QUIET)
if (NOT SDL3_FOUND)
  message(STATUS "Getting SDL3 from Github")
  if(WIN32)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
  endif()
  include_dependency(SDL3 https://github.com/libsdl-org/SDL.git release-3.2.18)
else()
  message(STATUS "Using local SDL3")
endif()

# C standard
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_FILE_OFFSET_BITS=64")

# Warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-missing-field-initializers")

function(Enable_Tests)
  set(TEST ON CACHE INTERNAL "")
endfunction()

function(Build_Demo)
  add_subdirectory(demo)
  include_directories(demo)
endfunction()

function(Build_Library)
  add_subdirectory(src)
  include_directories(src)
endfunction()

function(Build_Tests)
  add_subdirectory(external)
  add_subdirectory(test)
endfunction()

if(TEST)
  Enable_Tests()
  Build_Library()
  Build_Tests()
else()
  Build_Library()
  Build_Demo()
endif()
