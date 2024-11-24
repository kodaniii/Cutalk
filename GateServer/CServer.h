#pragma once

#include "defs.h"

//CRTP
class CServer: public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context&, unsigned short&);
	void start();
	
private:
	tcp::acceptor _acceptor;
	net::io_context& _ioc;
	//tcp::socket _socket;
};

