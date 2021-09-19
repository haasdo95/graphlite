target_sources(${PROJECT_NAME} INTERFACE deserialize.h)
target_include_directories(${PROJECT_NAME} AFTER INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
