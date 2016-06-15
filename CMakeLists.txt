cmake_minimum_required(VERSION 2.6)

project(slither_server)

message ("** " ${PROJECT_NAME})
# Set CMake library search policy
if (COMMAND cmake_policy)
    cmake_policy (SET CMP0011 NEW)
endif ()

# Include our cmake macros
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
include (cmake/cppcheck-target-cmake/FindCPPCHECK)
include (cmake/cppcheck-target-cmake/CPPCheck)

# Detect clang. Not officially reported by cmake.
execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-v" ERROR_VARIABLE CXX_VER_STDERR)
if ("${CXX_VER_STDERR}" MATCHES ".*clang.*")
    set (CMAKE_COMPILER_IS_CLANGXX 1)
endif ()

# Default CXX flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Visual studio
if (MSVC)
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL /Gy /GF /Ox /Ob2 /Ot /Oi /MP /arch:SSE2 /fp:fast")
    set (CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
    add_definitions (/W4 /W44640 /wd4996 /wd4995 /wd4355)
    add_definitions (-DUNICODE -D_UNICODE)
    add_definitions (-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
    add_definitions (-DNOMINMAX)
endif ()

# clang or gcc
if (CMAKE_COMPILER_IS_CLANGXX OR CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic")
endif ()

# Source & includes
file (GLOB_RECURSE SOURCE_FILES src/*.cpp)
file (GLOB_RECURSE HEADER_FILES src/*.hpp)

include_directories (src)
include_directories (third_party/websocketpp)

# Dependencies
set (Boost_FIND_REQUIRED TRUE)
set (Boost_FIND_QUIETLY FALSE)
set (Boost_DEBUG TRUE)
set (Boost_USE_MULTITHREADED TRUE)
set (Boost_ADDITIONAL_VERSIONS "1.39.0" "1.40.0" "1.41.0" "1.42.0" "1.43.0" "1.44.0" "1.46.1")

find_package (Boost 1.39.0 REQUIRED COMPONENTS system thread program_options)

if (Boost_FOUND)
    # Boost is a project wide global dependency.
    include_directories (${Boost_INCLUDE_DIRS})
    link_directories (${Boost_LIBRARY_DIRS})

    # Pretty print status
    message (STATUS "-- Include Directories")
    foreach (include_dir ${Boost_INCLUDE_DIRS})
        message (STATUS "       " ${include_dir})
    endforeach ()
    message (STATUS "-- Library Directories")
    foreach (library_dir ${Boost_LIBRARY_DIRS})
        message (STATUS "       " ${library_dir})
    endforeach ()
    message (STATUS "-- Libraries")
    foreach (boost_lib ${Boost_LIBRARIES})
        message (STATUS "       " ${boost_lib})
    endforeach ()
    message ("")
else ()
    message (FATAL_ERROR "Failed to find required dependency: boost")
endif ()

# find_package(ZLIB)

# Build
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES})
# target_link_libraries (${PROJECT_NAME} ${ZLIB_LIBRARIES})

set_target_properties (${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# CppCheck
cppcheck_target_sources (${PROJECT_NAME} INCLUDES ${INCLUDES})

