#pragma once
#include "defs.h"

class HttpConnection: public std::enable_shared_from_this<HttpConnection>
{
public:
	friend class LogicSystem;
	HttpConnection(tcp::socket);
	void start();

private:
	void checkTimeout();
	void writeResponse();
	void handleRequest(); 
	void PreParseGetParam();

	tcp::socket _socket;
	beast::flat_buffer _buffer{ TCP_BUFFER };

	http::request<http::dynamic_body> _request;
	http::response<http::dynamic_body> _response;

	net::steady_timer _timeout{
		_socket.get_executor(), std::chrono::seconds(TIME_OUT)
	};

	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
};

