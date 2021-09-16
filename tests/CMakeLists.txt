set(BINARY ${CMAKE_PROJECT_NAME}_tst)

SET(TEST_SRC main.cpp detail_test.cpp no_prop_test.cpp prop_test.cpp serializer_test.cpp test_utils.h)
SET(TEST_SRC_WITH_DOT ${TEST_SRC} deserializer_test.cpp)

if (GRAPH_LITE_SUPPORT_DOT)
    message("building test with dot support")
    add_executable(${BINARY} ${TEST_SRC_WITH_DOT})
    target_link_libraries(${BINARY} dot_parser)
else()
    message("building test without dot support")
    add_executable(${BINARY} ${TEST_SRC})
endif ()

set_target_properties(${BINARY} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)


add_test(NAME ${BINARY} COMMAND ${BINARY} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

target_link_libraries(${BINARY} gtest ${CMAKE_PROJECT_NAME})

set_target_properties(${BINARY} PROPERTIES LINKER_LANGUAGE CXX)
