#include "stdafx.h"
#include "shared.h"

using asio::ip::icmp;

void send_request(icmp::socket &socket, const icmp::endpoint &ep, icmp_header &header)
{
    header.type(icmp_header::timestamp_request);
    header.code(0);
    header.identifier(0);
    header.sequence_number(0);

    request_t orig_timestamp = asio::detail::socket_ops::host_to_network_long(pt::second_clock::universal_time().time_of_day().total_milliseconds());

    asio::streambuf buffer;
    icmp_encode(buffer, header, orig_timestamp);

    socket.send_to(buffer.data(), ep);
}

bool receive_reply(icmp::socket &socket, const icmp::endpoint &ep, const icmp_header &request_header)
{
    const size_t default_size = 0x1000;

    asio::streambuf buffer;

    size_t len = socket.receive(buffer.prepare(default_size));
    buffer.commit(len);            

    icmp_header header;
    reply_t data;

    icmp_decode(buffer, header, data);

    if (header.type() == icmp_header::timestamp_reply 
        && header.identifier() == request_header.identifier() 
        && header.sequence_number() == request_header.sequence_number())
    {
        const uint32_t ts1 = asio::detail::socket_ops::network_to_host_long(data[1]);
        const uint32_t ts2 = asio::detail::socket_ops::network_to_host_long(data[2]);
        
        cout << "Reply: " << pt::time_duration(pt::milliseconds(ts1)) << " - " << pt::time_duration(pt::milliseconds(ts2)) << endl;
        return true;    
    }

    return false;
}
    
int main()
{
    try
    {
        asio::streambuf buffer;
        
        io_service service;
        icmp::resolver resolver(service);
        icmp::resolver::query query(icmp::v4(), "192.168.209.128", "");
        icmp::endpoint destination = *resolver.resolve(query);

        icmp::socket socket(service, icmp::v4());

        icmp_header header;
        send_request(socket, destination, /*out*/ header);
        while (!receive_reply(socket, destination, header))
        {

        }
    }
    catch(std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
