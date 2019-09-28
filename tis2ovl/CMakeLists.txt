cmake_minimum_required(VERSION 3.1.0)
set(PROJECT_NAME "tis2ovl")
project (${PROJECT_NAME})

# Adding required libraries
set(PROJECT_LINK_LIBS libimagequant.a)
find_library(C_LIBRARIES ${PROJECT_LINK_LIBS} PATHS lib lib/libimagequant)

# make header files available to the build environment
include_directories(src lib lib/libimagequant)

# Add source files using wildcards
file(GLOB SOURCES "src/*.c")

# Alternatively, add files individually
# set(SOURCES src/main.c src/global.c ...)

# Global compiler flags
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -msse -mfpmath=sse")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu99 -msse -mfpmath=sse")

# Release output doesn't need debug symbols
if(NOT APPLE)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${C_LIBRARIES})

# math library required by libimagequant
target_link_libraries(${PROJECT_NAME} m)

# macOS: Debug symbols have to be stripped manually
if (CMAKE_BUILD_TYPE STREQUAL "Release" AND APPLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} -u -r $<TARGET_FILE:${PROJECT_NAME}>)
endif()
