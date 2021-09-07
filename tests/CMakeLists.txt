set(BINARY ${CMAKE_PROJECT_NAME}_tst)

add_executable(${BINARY} main.cpp detail_test.cpp no_prop_test.cpp prop_test.cpp serializer_test.cpp test_utils.h)

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} gtest ${CMAKE_PROJECT_NAME})

set_target_properties(${BINARY} PROPERTIES LINKER_LANGUAGE CXX)
