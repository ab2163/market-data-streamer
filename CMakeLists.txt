cmake_minimum_required(VERSION 3.24)
project(mbo_replay)

include(FetchContent)
FetchContent_Declare(
    databento
    GIT_REPOSITORY https://github.com/databento/databento-cpp
    GIT_TAG main
)
FetchContent_MakeAvailable(databento)

add_executable(data_streamer 
    src/data_streamer.cpp
    src/server.cpp
)
target_link_libraries(data_streamer PRIVATE databento::databento)

add_executable(book_constructor
    src/book_constructor.cpp
    src/client.cpp
)
target_link_libraries(book_constructor PRIVATE databento::databento)
