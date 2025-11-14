cmake_minimum_required(VERSION 3.24)
project(mbo_replay)

include(FetchContent)
FetchContent_Declare(
    databento
    GIT_REPOSITORY https://github.com/databento/databento-cpp
    GIT_TAG main
)
FetchContent_MakeAvailable(databento)

add_executable(run_streamer 
    src/run_streamer.cpp
    src/data_streamer.cpp
    src/server.cpp
    src/thread_pool.cpp
    src/socket.cpp
    src/message_connection.cpp
)
target_link_libraries(run_streamer PRIVATE databento::databento)

add_executable(run_builder
    src/run_builder.cpp
    src/book_constructor.cpp
    src/client.cpp
    src/socket.cpp
    src/message_connection.cpp
)
target_link_libraries(run_builder PRIVATE databento::databento)
