#pragma once

#include <cstddef>

namespace cfg{
    inline constexpr size_t BATCH_SIZE = 500; //size of batches of messages to send over TCP frames
    inline constexpr size_t MAX_QUEUE_SIZE = 50000;
    inline constexpr int DEFAULT_PORT = 9000;
    inline constexpr const char* DEFAULT_HOST = "127.0.0.1";
    inline constexpr int SERVER_SEND_TIMEOUT_SEC = 2;
    inline constexpr int SERVER_RECV_TIMEOUT_SEC = 30;
    inline constexpr int CLIENT_SEND_TIMEOUT_SEC = 10;
    inline constexpr int CLIENT_RECV_TIMEOUT_SEC = 30;
    inline constexpr int PRINT_FREQ = 200; //frequency of timestamps over which to print output data
}
