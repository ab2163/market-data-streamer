cmake_minimum_required(VERSION 3.24)
project(mbo_replay)

include(FetchContent)
FetchContent_Declare(
    databento
    GIT_REPOSITORY https://github.com/databento/databento-cpp
    GIT_TAG        main
)
FetchContent_MakeAvailable(databento)

add_executable(replay main.cpp)
target_link_libraries(replay PRIVATE databento::databento)

