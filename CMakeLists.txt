cmake_minimum_required(VERSION 3.31.6)
project("libraycast" C)
include(CTest)

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
    include_dependency(SDL3 https://github.com/libsdl-org/SDL.git release-3.2.18)
else()
    message(STATUS "Using local SDL3")
endif()


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

if(TARGET_GROUP STREQUAL demo)
  Build_Library()
  Build_Demo()
elseif(TARGET_GROUP STREQUAL test)
  Enable_Tests()
  Build_Library()
  Build_Tests()
elseif(TARGET_GROUP STREQUAL all)
  Enable_Tests()
  Build_Library()
  Build_Demo()
  Build_Tests()
else()
  Build_Library()
endif()
