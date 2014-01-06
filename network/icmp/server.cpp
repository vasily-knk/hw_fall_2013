#include "stdafx.h"
#include "shared.h"

using asio::ip::icmp;

bool receive_request(icmp::socket &socket, icmp::endpoint &ep, icmp_header &header, request_t &data)
{
    const size_t default_size = 0x1000;

    asio::streambuf buffer;

    size_t len = socket.receive_from(buffer.prepare(default_size), ep);
    buffer.commit(len);            

    icmp_decode(buffer, header, data);

    if (header.type() == icmp_header::timestamp_request)
        return true;

    return false;
}

void send_reply(icmp::socket &socket, const icmp::endpoint &ep, const icmp_header &request_header, const request_t &request_data)
{
    icmp_header header;
    header.type(icmp_header::timestamp_reply);
    header.code(0);
    header.identifier(request_header.identifier());
    header.sequence_number(request_header.sequence_number());

    reply_t data;
    data[0] = request_data;
    data[1] = asio::detail::socket_ops::host_to_network_long(pt::second_clock::universal_time().time_of_day().total_milliseconds());
    data[2] = asio::detail::socket_ops::host_to_network_long(pt::second_clock::universal_time().time_of_day().total_milliseconds());

    asio::streambuf buffer;
    icmp_encode(buffer, header, data);

    socket.send_to(buffer.data(), ep);
}

int server_main()
{
    const size_t default_size = 65536;
    
    try
    {
        io_service service;
        icmp::socket socket(service, icmp::v4());
    

        while(true)
        {
            icmp::endpoint ep;
            icmp_header request_header;
            request_t data;

            if (receive_request(socket, /*out*/ ep, /*out*/ request_header, /*out*/ data))
            {
                cout << "Request: " << data << endl;
                send_reply(socket, ep, request_header, data);
            }
        }
    }
    catch(std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
