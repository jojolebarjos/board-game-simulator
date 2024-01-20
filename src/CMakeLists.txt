cmake_minimum_required(VERSION 3.15)

add_executable(foo foo.cpp)

target_link_libraries(foo
	PRIVATE
		game
)
