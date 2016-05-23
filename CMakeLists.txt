cmake_minimum_required(VERSION 2.6)

project(slither_server)

message ("** " ${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file (GLOB_RECURSE SOURCE_FILES src/*.cpp)
file (GLOB_RECURSE HEADER_FILES src/*.hpp)

include_directories (src)
include_directories (third_party/websocketpp)

set (Boost_FIND_REQUIRED TRUE)
set (Boost_FIND_QUIETLY TRUE)
set (Boost_DEBUG FALSE)
set (Boost_USE_MULTITHREADED TRUE)
set (Boost_ADDITIONAL_VERSIONS "1.39.0" "1.40.0" "1.41.0" "1.42.0" "1.43.0" "1.44.0" "1.46.1")

find_package (Boost 1.39.0 REQUIRED COMPONENTS system thread)

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

find_package(ZLIB)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries (${PROJECT_NAME} ${ZLIB_LIBRARIES})

set_target_properties (${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
