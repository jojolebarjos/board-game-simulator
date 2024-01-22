cmake_minimum_required(VERSION 3.15)
project(game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

add_library(game INTERFACE)
target_include_directories(game INTERFACE include/)

# TODO add as option (and also add define in code)
target_link_libraries(game INTERFACE nlohmann_json::nlohmann_json)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()
    add_subdirectory(src)
    add_subdirectory(tests)
endif()
