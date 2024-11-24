#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
	: _ioc(ioc)
	, _acceptor(ioc, tcp::endpoint(tcp::v4(), port)){
	//, _socket(ioc){
}

void CServer::start() {
	auto self = shared_from_this();

	//ioc pool
	//引用：io_context不支持拷贝构造和拷贝赋值
	auto &get_ioc = AsioIOServicePool::GetInstance()->GetIOService();	//GetNextIOContext
	std::shared_ptr<HttpConnection> new_conn = std::make_shared<HttpConnection>(get_ioc);

	//异步监听+回调
	_acceptor.async_accept(new_conn->GetSocket(), [self, new_conn](beast::error_code ec) {
		try {
			//error, 放弃当前监听, 监听其他socket
			if (ec) {
				self->start();
				return;
			}

			//创建HttpConnection类，该类管理当前链接
			//TODO HttpConnection
			//std::make_shared<HttpConnection>(std::move(self->_socket))->start();
			new_conn->start();

			self->start();
		}
		catch (std::exception& exp) {
			std::cout << "exception is " << exp.what() << std::endl;
		}
	});
}

