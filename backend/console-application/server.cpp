#include <ctime>
#include <iostream>
#include <cstring>
#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "Console.h"
using namespace boost::asio::ip;

//on Windows: -lws2_32 -lwsocks32
const std::string rem_address = "127.0.0.1";
boost::asio::ip::address add;
tcp::endpoint endpoint;

sjtu::Console *console;

class session
{
private:
    static const int max_length = 10240;
    char data[max_length];
    char output[max_length];
    tcp::socket _socket;

public:
    session(boost::asio::io_service &service_) : _socket(service_) {}
    tcp::socket& socket() { return _socket; }

public:
    void start()
    {
        _socket.async_read_some(boost::asio::buffer(data, max_length), boost::bind(&session::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void handle_read(const boost::system::error_code &error, size_t bytes_transferred)
    {
        if (!error)
        {
        	for (int i = bytes_transferred; i < max_length; i++) data[i] = 0;
            std::cerr << "MESSAGE BEGIN" << " " << bytes_transferred << std::endl;
            for (int i = 0; i < bytes_transferred; i++) std::cerr << (int)data[i]<<" "; std::cerr << std::endl;
            std::cerr << data << std::endl;
			std::cerr << std::endl << "End" << std::endl;

            /* DEALS WITH COMMAND HERE */
			try
			{
				std::istringstream is(data);
				std::cerr << is.str() << std::endl;
				std::ostringstream os;
				console->processline(is, os);

				memset(output, 0, sizeof(output));
				strcpy(output, os.str().c_str());

				std::cerr << "Return: " << output << std::endl;
				std::cerr << "(Len: " << strlen(output) << ")" << std::endl;
			}
			catch(...)
			{
				memset(output, 0, sizeof(output));
			}

            boost::asio::async_write(_socket, boost::asio::buffer(output, strlen(output)), boost::bind(&session::handle_write, this, boost::asio::placeholders::error));
        }
        else delete this;
    }

    void handle_write(const boost::system::error_code &error)
    {
        delete this;
    }
};

class server
{
private:
    boost::asio::io_service &io_service;
    tcp::acceptor acceptor;

public:
    server(boost::asio::io_service &io_service_) : io_service(io_service_), acceptor(io_service, endpoint)
    {
        auto new_session = new session(io_service);
        acceptor.async_accept(new_session->socket(), boost::bind(&server::handle_accept, this, new_session, boost::asio::placeholders::error));
    }

public:
    void handle_accept(session *s, const boost::system::error_code &error)
    {
       if (!error)
        {
            s->start();
            s = new session(io_service);
            acceptor.async_accept(s->socket(), boost::bind(&server::handle_accept, this, s, boost::asio::placeholders::error));
        }
        else delete s;
    }

};

int main()
{
	sjtu::logRecovery.Redo();
	sjtu::logRecovery.Undo();
	sjtu::Console t;
	console = &t;

	std::cout << "Server start" << std::endl;
    try
    {
        boost::asio::io_service io_service;
        add.from_string(rem_address);
        endpoint = tcp::endpoint(add, (short)9001);

        server s(io_service);
        io_service.run();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
