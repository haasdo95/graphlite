target_sources(${CMAKE_PROJECT_NAME} INTERFACE deserialize.h)
target_include_directories(${CMAKE_PROJECT_NAME} AFTER INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
