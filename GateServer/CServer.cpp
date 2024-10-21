#include "CServer.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
	: _ioc(ioc)
	, _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
	, _socket(ioc){

}

void CServer::start() {
	auto self = shared_from_this();

	//异步监听+回调
	_acceptor.async_accept(_socket, [self](beast::error_code ec) {
		try {
			//error, 放弃当前监听, 监听其他socket
			if (ec) {
				self->start();
				return;
			}

			//创建HttpConnection类，该类管理当前链接
			//TODO HttpConnection
			std::make_shared<HttpConnection>(std::move(_socket))->start();
			
			self->start();
		}
		catch (std::exception& exp) {
			
		}
		});
}

