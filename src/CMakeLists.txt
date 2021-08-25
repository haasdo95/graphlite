add_executable(${CMAKE_PROJECT_NAME}_run graph_lite.h)
set_target_properties(${CMAKE_PROJECT_NAME}_run PROPERTIES LINKER_LANGUAGE CXX)

add_library(${CMAKE_PROJECT_NAME} SHARED graph_lite.h)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
