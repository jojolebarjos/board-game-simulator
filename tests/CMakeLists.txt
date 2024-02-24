cmake_minimum_required(VERSION 3.15)

include(FetchContent)
FetchContent_Declare(
	doctest
	GIT_REPOSITORY https://github.com/onqtam/doctest.git
	GIT_TAG 2.4.6
)
FetchContent_MakeAvailable(doctest)

function(add_game_test TEST_NAME TEST_SOURCE)
	add_executable(${TEST_NAME} ${TEST_SOURCE})
	target_link_libraries(${TEST_NAME} PRIVATE game doctest::doctest)
	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

add_game_test(test_connect connect.cpp)
add_game_test(test_tensor tensor.cpp)
