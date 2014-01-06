#pragma once

#include "ipv4_header.hpp"
#include "icmp_header.hpp"

typedef uint32_t request_t;
typedef std::array<uint32_t, 3> reply_t;

template<typename T>
void icmp_decode(asio::streambuf &buffer, icmp_header &header, T &data)
{
    const size_t size = sizeof(T);

    auto data_ptr = reinterpret_cast<char*>(&data);
    
    ipv4_header ipv4_hdr;
    std::istream is(&buffer);
    is >> ipv4_hdr >> header;
    is.read(data_ptr, size);
}

template<typename T>
void icmp_encode(asio::streambuf &buffer, const icmp_header &src_header, const T &data)
{
    const size_t size = sizeof(T);
    
    icmp_header header = src_header;
    auto data_ptr = reinterpret_cast<const char*>(&data);
    compute_checksum(header, data_ptr, data_ptr + size);

    std::ostream os(&buffer);
    os << header;
    os.write(data_ptr, size);
}


