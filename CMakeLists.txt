cmake_minimum_required(VERSION 2.6)

project(slither_server)

message ("** " ${PROJECT_NAME})
# Set CMake library search policy
if (COMMAND cmake_policy)
    cmake_policy (SET CMP0011 NEW)
endif ()

# Include our cmake macros
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cppcheck-target-cmake/FindCPPCHECK")
    include (cmake/cppcheck-target-cmake/FindCPPCHECK)
    include (cmake/cppcheck-target-cmake/CPPCheck)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/StyleCheck.cmake")
    include (cmake/StyleCheck)
endif()

# Detect clang. Not officially reported by cmake.
execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-v" ERROR_VARIABLE CXX_VER_STDERR)
if ("${CXX_VER_STDERR}" MATCHES ".*clang.*")
    set (CMAKE_COMPILER_IS_CLANGXX 1)
endif ()

# Default CXX flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wshadow -Werror -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-unused-parameter -Wno-unused-function")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math -funroll-loops -march=native -O3")

# for old clang 3.5, 3.6 vs gcc 4.8 includes
# need for ubuntu 14.04
# don't need for ubuntu 16.04
# if (CMAKE_COMPILER_IS_CLANGXX)
#    set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-stdlib=libc++")
# endif ()

# Source & includes
file (GLOB_RECURSE SOURCE_FILES src/*.cc)
file (GLOB_RECURSE HEADER_FILES src/*.h)
set (FILES ${SOURCE_FILES} ${HEADER_FILES})

include_directories (src)
include_directories (third_party/websocketpp)

# Dependencies
set (Boost_FIND_REQUIRED TRUE)
set (Boost_FIND_QUIETLY TRUE)
set (Boost_DEBUG FALSE)
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

# OpenSSL for TLS/WSS support
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
endif()

# Build
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
# target_link_libraries (${PROJECT_NAME} ${ZLIB_LIBRARIES})

set_target_properties (${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# CppCheck
if(COMMAND cppcheck_target_sources)
    cppcheck_target_sources (${PROJECT_NAME})
endif()

# StyleCheck
if(COMMAND add_style_check_target)
    add_style_check_target (${PROJECT_NAME} "${FILES}")
endif()
