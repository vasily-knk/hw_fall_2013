#include "stdafx.h"
#include "server.h"

int main()
{
    using boost::asio::ip::tcp;

    try
    {
        boost::asio::io_service io_service;
        tcp_server server(io_service);
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}