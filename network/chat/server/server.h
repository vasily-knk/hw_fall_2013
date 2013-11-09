#pragma once

struct tcp_connection;
typedef shared_ptr<tcp_connection> tcp_connection_ptr;

struct tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service);
    ~tcp_server();

private:
    void start_accept();
    void handle_accept(tcp_connection_ptr new_connection, const boost::system::error_code& error);

private:
    tcp::acceptor acceptor_;
};

