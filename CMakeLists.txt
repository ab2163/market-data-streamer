cmake_minimum_required(VERSION 3.24)
project(mbo_replay)

enable_testing()

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

add_executable(test_order_book
    tests/test_order_book.cpp
    src/order_book.cpp
)
target_link_libraries(test_order_book
    PRIVATE
        databento::databento
        Catch2::Catch2WithMain
)

add_executable(test_message_connection
    tests/test_message_connection.cpp
    src/message_connection.cpp
    src/socket.cpp
    src/server.cpp
    src/thread_pool.cpp
)
target_link_libraries(test_message_connection
    PRIVATE
        databento::databento
        Catch2::Catch2WithMain
)

add_executable(test_single_client
    tests/test_single_client.cpp
    src/message_connection.cpp
    src/socket.cpp
    src/server.cpp
    src/thread_pool.cpp
    src/client.cpp
    src/order_book.cpp
    src/data_streamer.cpp
    src/book_constructor.cpp
)
target_link_libraries(test_single_client
    PRIVATE
        databento::databento
        Catch2::Catch2WithMain
)

add_executable(test_multi_client
    tests/test_multi_client.cpp
    src/message_connection.cpp
    src/socket.cpp
    src/server.cpp
    src/thread_pool.cpp
    src/client.cpp
    src/order_book.cpp
    src/data_streamer.cpp
    src/book_constructor.cpp
)
target_link_libraries(test_multi_client
    PRIVATE
        databento::databento
        Catch2::Catch2WithMain
)

add_test(NAME test_order_book COMMAND test_order_book)
add_test(NAME test_message_connection COMMAND test_message_connection)
add_test(NAME test_single_client COMMAND test_single_client)
add_test(NAME test_multi_client COMMAND test_multi_client)

