add_library(${PROJECT_NAME} INTERFACE)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(graph_lite)

if (GRAPH_LITE_SUPPORT_DOT)
    target_link_libraries(${PROJECT_NAME} INTERFACE dot_parser)
    add_subdirectory(dot_support)
endif ()