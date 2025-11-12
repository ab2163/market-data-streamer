#pragma once
#include "tcp_common.hpp"

//function which writes n bytes over the connection
inline void write_n(int file_desc, const void *buf, size_t n){
    const uint8_t *p = static_cast<const uint8_t*>(buf);
    size_t bytes_sent = 0;
    while(bytes_sent < n){
        ssize_t return_code = ::send(file_desc, p + bytes_sent, n - bytes_sent, 0);
        if(return_code < 0){
            if(errno == EINTR) continue;
            throw std::system_error(errno, std::generic_category(), "Error in sending over TCP");
        }
        if(return_code == 0) throw std::runtime_error("Error: connection closed by peer");
        bytes_sent += static_cast<size_t>(return_code);
    }
}

//read n bytes over the connection
inline bool read_n(int file_desc, void* buf, size_t n){
    uint8_t* p = static_cast<uint8_t*>(buf);
    size_t bytes_recv = 0;
    while(bytes_recv < n){
        ssize_t return_code = ::recv(file_desc, p + bytes_recv, n - bytes_recv, 0);
        if(return_code < 0){
            if (errno == EINTR) continue;
            throw std::system_error(errno, std::generic_category(), "Error in receiving over TCP");
        }
        if(return_code == 0) return false;
        bytes_recv += static_cast<size_t>(return_code);
    }
    return true;
}

//send data as length-prefixed frame: [u32 length_le][payload bytes]
inline void send_frame(int file_desc, const void *data, uint32_t len){
    //use little-endian consistently
    uint32_t len_LE = htole32(len);
    write_n(file_desc, &len_LE, sizeof(len_LE));
    write_n(file_desc, data, len);
}

//receive data as length-prefixed frame
inline bool recv_frame(int file_desc, void *buf){
    uint32_t len_LE = 0;
    if(!read_n(file_desc, &len_LE, sizeof(len_LE))) return false;;
    uint32_t len = le32toh(len_LE);
    if(len > (64u << 20)) //64 MiB sanity cap
        throw std::runtime_error("Frame too large");
    if(len) return read_n(file_desc, buf, len);
    else return false;
}