cmake_minimum_required(VERSION 3.20)
project(graphlite)

set(CMAKE_CXX_STANDARD 17)

# support dot language
option(GRAPH_LITE_SUPPORT_DOT "adding support for the dot language" ON)

if (GRAPH_LITE_SUPPORT_DOT)
    message("DOT language support added")
    add_subdirectory(lib/dot_parser_cpp)
endif ()

add_subdirectory(src)

# testing dependency
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(GRAPH_LITE_BUILD_TESTS "whether or not tests should be built" ON)
    if (GRAPH_LITE_BUILD_TESTS)
        include(CTest)
        enable_testing()
        add_subdirectory(tests)
        add_subdirectory(lib/googletest)
    endif()
endif()