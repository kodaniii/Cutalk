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
			
			//ignore bytes
			boost::ignore_unused(bytes);
			
			self->handleRequest();
			self->checkTimeout();
		}
		catch (std::exception& exp) {
			std::cout << "exception is " << exp.what() << std::endl;
		}
	});
}

void HttpConnection::handleRequest() {
	_response.version(_request.version());
	_response.keep_alive(false);
	
	if (_request.method() == http::verb::get) {
		bool success = LogicSystem::getInstance()->handleGet(_request.target(), shared_from_this());
		if (!success) {
			_response.result(http::status::not_found);
			_response.set(http::field::content_type, "text/plain");
			beast::ostream(_response.body()) << "url not found\r\n";
			writeResponse();
			return;
		}
		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");
		writeResponse();
		return;
	}
}

void HttpConnection::writeResponse() {
	auto self = shared_from_this();
	_response.content_length(_response.body().size());
	http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t bytes) {
		self->_socket.shutdown(tcp::socket::shutdown_send, ec);
		self->_timeout.cancel();
		}
	);
}

void HttpConnection::checkTimeout() {
	auto self = shared_from_this();
	_timeout.async_wait([self](beast::error_code ec) {
		if (!ec) {
			self->_socket.close();
		}
	});
}