#pragma once

#include "defs.h"
#include "CSession.h"

class CServer
{
public:
	CServer(boost::asio::io_context&, short);
	~CServer();
	void ClearSession(std::string);
	
private:
	void HandleAccept(shared_ptr<CSession>, const boost::system::error_code& error);
	void Start();

	boost::asio::io_context& _ioc;
	tcp::acceptor _acceptor;
	std::map<std::string, shared_ptr<CSession>> _sessions;
	std::mutex _mtx;
};

