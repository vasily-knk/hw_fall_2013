#include "stdafx.h"
#include "server.h"

using boost::asio::ip::tcp;

std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

struct tcp_connection
    : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef tcp_connection_ptr pointer;

    static pointer create(boost::asio::io_service& io_service)
    {
        return pointer(new tcp_connection(io_service));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        boost::asio::async_read(socket_,    )
        
        message_ = make_daytime_string();

        boost::asio::async_write(socket_, boost::asio::buffer(message_),
            boost::bind(&tcp_connection::handle_write, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

private:
    tcp_connection(boost::asio::io_service& io_service)
        : socket_(io_service)
    {
    }

    void handle_write(const boost::system::error_code& /*error*/,
        size_t /*bytes_transferred*/)
    {
    }

    tcp::socket socket_;
    std::string message_;
};


tcp_server::tcp_server(boost::asio::io_service& io_service)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), 13))
{
    start_accept();
}

tcp_server::~tcp_server()
{

}

void tcp_server::start_accept()
{
    tcp_connection::pointer new_connection =
        tcp_connection::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
        boost::asio::placeholders::error));
}

void tcp_server::handle_accept(tcp_connection::pointer new_connection,
    const boost::system::error_code& error)
{
    if (!error)
    {
        new_connection->start();
    }

    start_accept();
}


