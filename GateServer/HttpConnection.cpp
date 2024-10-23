#include "HttpConnection.h"

HttpConnection::HttpConnection(tcp::socket socket)
	: _socket(std::move(socket)){

}

void HttpConnection::start() {
	auto self = shared_from_this();
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, std::size_t bytes) {
		try {
			if (ec) {
				std::cout << "http read err is " << ec.what() << std::endl;
				return;
			}
			
			boost::ignore_unused(bytes);
			//TODO 
			//self->handleRequest();
			//self->checkTimeout();
		}
		catch (std::exception& exp) {
			std::cout << "exception is " << exp.what() << std::endl;
		}
	});
}