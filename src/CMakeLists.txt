project("libraycast" C)

set(LIBRARY_NAME "raycast")
set(LIBRARY_VERSION_MAJOR 0)
set(LIBRARY_VERSION_STRING 0.0)

# C standard
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_FILE_OFFSET_BITS=64")

# Warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-missing-field-initializers")


include(GNUInstallDirs)

set(LIBRARY_BASE_PATH ".")

INCLUDE_DIRECTORIES (
	"${LIBRARY_BASE_PATH}"
)

set(LIBRARY_PUBLIC_SRC
	"${LIBRARY_BASE_PATH}/raycast/raycast.c"
)

set(LIBRARY_PUBLIC_HEADERS
	"${LIBRARY_BASE_PATH}/raycast/raycast.h"
)

add_library (
	${LIBRARY_NAME} SHARED ${LIBRARY_PUBLIC_SRC}
)

target_link_libraries(${LIBRARY_NAME} SDL3::SDL3 m)

set_target_properties(
	${BINARY_NAME} PROPERTIES
	VERSION		${LIBRARY_VERSION_STRING}
	SOVERSION	${LIBRARY_VERSION_MAJOR}
	PUBLIC_HEADER  ${LIBRARY_PUBLIC_HEADERS}
)


# Compiler definitions
ADD_DEFINITIONS("-g")
if(TEST)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTEST")
endif()

# Library installation directives
INSTALL (
	TARGETS ${BINARY_NAME}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
