cmake_minimum_required(VERSION 3.24)
project(mbo_replay)

include(FetchContent)
FetchContent_Declare(
    databento
    GIT_REPOSITORY https://github.com/databento/databento-cpp
    GIT_TAG main
)
FetchContent_MakeAvailable(databento)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.11.0
)
FetchContent_MakeAvailable(Catch2)

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
    src/server.cpp
    src/thread_pool.cpp
    src/order_book.cpp
)
target_link_libraries(run_builder PRIVATE databento::databento)

add_executable(order_book_tests
    tests/order_book_tests.cpp
    src/order_book.cpp
)
target_link_libraries(order_book_tests
    PRIVATE
        databento::databento
        Catch2::Catch2WithMain
)
