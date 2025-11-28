#include "../src/message_connection.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <catch2/catch_test_macros.hpp>

//helper to create a connected socket pair
static void make_socket_pair(int fds[2]){
    int rc = ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    REQUIRE(rc == 0);
}

//helper to construct a minimal MboMsg
static MboMsg make_msg(uint64_t id){
    MboMsg m{};
    m.order_id = id;
    return m;
}

TEST_CASE("push_onto_queue accepts messages within capacity and updates counters"){
    MessageConnection conn;

    vector<MboMsg> batch;
    batch.reserve(cfg::BATCH_SIZE);
    for(size_t i = 0; i < cfg::BATCH_SIZE; ++i){
        batch.push_back(make_msg(i + 1));
    }

    REQUIRE(conn.to_send.empty());
    REQUIRE(conn.loss_send == 0);
    REQUIRE(conn.msgs_inp == 0);

    bool ok = conn.push_onto_queue(batch);
    REQUIRE(ok);
    REQUIRE(conn.to_send.size() == batch.size());
    REQUIRE(conn.loss_send == 0);
    REQUIRE(conn.msgs_inp == static_cast<int>(batch.size()));
}

TEST_CASE("push_onto_queue drops messages and increments loss_send when queue would overflow"){
    MessageConnection conn;

    //pre-fill queue close to MAX_QUEUE_SIZE
    size_t prefill = cfg::MAX_QUEUE_SIZE;
    conn.to_send.assign(prefill, make_msg(1));

    vector<MboMsg> batch(10, make_msg(2)); //any non-zero number

    int prev_loss = conn.loss_send;
    int prev_inp = conn.msgs_inp;

    bool ok = conn.push_onto_queue(batch);
    REQUIRE_FALSE(ok);
    REQUIRE(conn.to_send.size() == prefill); //unchanged
    REQUIRE(conn.loss_send == prev_loss + static_cast<int>(batch.size()));
    REQUIRE(conn.msgs_inp == prev_inp); //not incremented on failure
}

TEST_CASE("send_messages sends at most BATCH_SIZE messages and leaves remainder in queue"){
    int fds[2];
    make_socket_pair(fds);

    //use server-side constructor with nullptr server (we will never call with last=true)
    MessageConnection conn(fds[0], nullptr);

    //fill queue with more than one batch
    size_t total_msgs = cfg::BATCH_SIZE + 5;
    for(size_t i = 0; i < total_msgs; ++i){
        conn.to_send.push_back(make_msg(i + 1));
    }

    REQUIRE(conn.to_send.size() == total_msgs);

    //send a single batch
    conn.send_messages(false);

    //after sending, queue should have total_msgs - BATCH_SIZE left
    REQUIRE(conn.to_send.size() == total_msgs - cfg::BATCH_SIZE);

    //verify frame on the other end of the socket
    uint32_t len_LE = 0;
    //use the same helper semantics as read_n, but here we can call ::read in a loop
    size_t bytes_read = 0;
    uint8_t *p = reinterpret_cast<uint8_t*>(&len_LE);
    while(bytes_read < sizeof(len_LE)){
        ssize_t rc = ::read(fds[1], p + bytes_read, sizeof(len_LE) - bytes_read);
        REQUIRE(rc > 0);
        bytes_read += static_cast<size_t>(rc);
    }

    uint32_t len = le32toh(len_LE);
    REQUIRE(len == cfg::BATCH_SIZE * sizeof(MboMsg));

    //read payload
    vector<MboMsg> payload(cfg::BATCH_SIZE);
    bytes_read = 0;
    uint8_t *q = reinterpret_cast<uint8_t*>(payload.data());
    while(bytes_read < len){
        ssize_t rc = ::read(fds[1], q + bytes_read, len - bytes_read);
        REQUIRE(rc > 0);
        bytes_read += static_cast<size_t>(rc);
    }

    //first BATCH_SIZE order_ids should have been sent
    for(size_t i = 0; i < cfg::BATCH_SIZE; ++i){
        REQUIRE(payload[i].order_id == i + 1);
    }

    ::close(fds[0]);
    ::close(fds[1]);
}

TEST_CASE("recv_onto_queue receives a frame and appends to from_server, updating msgs_sent"){
    int fds[2];
    make_socket_pair(fds);

    MessageConnection conn(fds[0], nullptr);

    //prepare a small batch of messages to send into the connection
    size_t num_msgs = std::min<size_t>(4, cfg::BATCH_SIZE);
    vector<MboMsg> msgs;
    msgs.reserve(num_msgs);
    for(size_t i = 0; i < num_msgs; ++i){
        msgs.push_back(make_msg(100 + i));
    }

    //write length-prefixed frame on the peer side
    uint32_t len = static_cast<uint32_t>(num_msgs * sizeof(MboMsg));
    uint32_t len_LE = htole32(len);

    //send length
    {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&len_LE);
        size_t bytes_sent = 0;
        while(bytes_sent < sizeof(len_LE)){
            ssize_t rc = ::write(fds[1], p + bytes_sent, sizeof(len_LE) - bytes_sent);
            REQUIRE(rc > 0);
            bytes_sent += static_cast<size_t>(rc);
        }
    }

    //send payload
    {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(msgs.data());
        size_t bytes_sent = 0;
        while(bytes_sent < len){
            ssize_t rc = ::write(fds[1], p + bytes_sent, len - bytes_sent);
            REQUIRE(rc > 0);
            bytes_sent += static_cast<size_t>(rc);
        }
    }

    REQUIRE(conn.from_server.empty());
    REQUIRE(conn.msgs_sent == 0);
    REQUIRE(conn.loss_recv == 0);

    bool ok = conn.recv_onto_queue();
    REQUIRE(ok);

    REQUIRE(conn.from_server.size() == num_msgs);
    REQUIRE(conn.msgs_sent == static_cast<int>(num_msgs));
    REQUIRE(conn.loss_recv == 0);

    for(size_t i = 0; i < num_msgs; ++i){
        REQUIRE(conn.from_server[i].order_id == 100 + i);
    }

    ::close(fds[0]);
    ::close(fds[1]);
}

TEST_CASE("recv_onto_queue drops messages and increments loss_recv when queue would overflow"){
    int fds[2];
    make_socket_pair(fds);

    MessageConnection conn(fds[0], nullptr);

    //pre-fill from_server close to MAX_QUEUE_SIZE
    size_t prefill = cfg::MAX_QUEUE_SIZE;
    conn.from_server.assign(prefill, make_msg(1));
    conn.msgs_sent = 0;
    conn.loss_recv = 0;

    //send a small valid frame that would overflow the queue
    size_t num_msgs = std::min<size_t>(4, cfg::BATCH_SIZE);
    vector<MboMsg> msgs(num_msgs, make_msg(2));

    uint32_t len = static_cast<uint32_t>(num_msgs * sizeof(MboMsg));
    uint32_t len_LE = htole32(len);

    //send length
    {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&len_LE);
        size_t bytes_sent = 0;
        while(bytes_sent < sizeof(len_LE)){
            ssize_t rc = ::write(fds[1], p + bytes_sent, sizeof(len_LE) - bytes_sent);
            REQUIRE(rc > 0);
            bytes_sent += static_cast<size_t>(rc);
        }
    }

    //send payload
    {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(msgs.data());
        size_t bytes_sent = 0;
        while(bytes_sent < len){
            ssize_t rc = ::write(fds[1], p + bytes_sent, len - bytes_sent);
            REQUIRE(rc > 0);
            bytes_sent += static_cast<size_t>(rc);
        }
    }

    bool ok = conn.recv_onto_queue();
    REQUIRE_FALSE(ok);

    //queue should be unchanged
    REQUIRE(conn.from_server.size() == prefill);
    REQUIRE(conn.msgs_sent == 0);
    REQUIRE(conn.loss_recv == static_cast<int>(num_msgs));

    ::close(fds[0]);
    ::close(fds[1]);
}
