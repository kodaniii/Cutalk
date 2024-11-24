#include "HttpConnection.h"
#include "LogicSystem.h"
#include "AsioIOServicePool.h"

HttpConnection::HttpConnection(boost::asio::io_context& ioc)//tcp::socket socket)
	: _socket(ioc){//std::move(socket)){
}

void HttpConnection::start() {
	auto self = shared_from_this();

	//ioc pool
	auto& get_ioc = AsioIOServicePool::GetInstance()->GetIOService();
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, std::size_t bytes) {
		try {
			if(ec && ec != http::error::end_of_stream) {
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

tcp::socket& HttpConnection::GetSocket() {
	return _socket;
}

void HttpConnection::handleRequest() {
	_response.version(_request.version());
	_response.keep_alive(false);
	
	if (_request.method() == http::verb::get) {
		//bool success = LogicSystem::GetInstance()->HandleGet(_request.target(), shared_from_this());
		PreParseGetParam();
		bool success = LogicSystem::GetInstance()->HandleGet(this->_get_url, shared_from_this());
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

	if (_request.method() == http::verb::post) {
		bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());
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

//10进制 -> 16进制
unsigned char ToHex(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}

//16进制 -> 10进制
unsigned char FromHex(unsigned char x)
{
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

std::string UrlEncode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//判断是否仅有数字和字母构成
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ') //为空字符
			strTemp += "+";
		else
		{
			//其他字符需要提前加%并且高四位和低四位分别转为16进制
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] & 0x0F);
		}
	}
	return strTemp;
}

std::string UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//还原+为空
		if (str[i] == '+') strTemp += ' ';
		//遇到%将后面的两个字符从16进制转为char再拼接
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high * 16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}

void HttpConnection::PreParseGetParam() {
	// 提取 URI
	//get_test?key1=value&key2=value2
	auto uri = _request.target();	//std::string
	// 查找查询字符串的开始位置（即'?'的位置） 
	auto query_pos = uri.find('?');	//size_t
	//not found ?
	if (query_pos == std::string::npos) {
		_get_url = uri;
		return;
	}
	_get_url = uri.substr(0, query_pos);	//(start idx, size_t)

	std::string query_string = uri.substr(query_pos + 1);	//query_pos --> ?
	std::string key;
	std::string value;
	size_t pos = 0;
	while ((pos = query_string.find('&')) != std::string::npos) {
		auto _pair = query_string.substr(0, pos);	//key1=value
		size_t eq_pos = _pair.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(_pair.substr(0, eq_pos)); // url_decode
			value = UrlDecode(_pair.substr(eq_pos + 1));	//start pos: eq_pos + 1
			_get_params[key] = value;
		}
		query_string.erase(0, pos + 1);
	}
	// 处理最后一个参数对（如果没有 & 分隔符）  
	if (!query_string.empty()) {
		size_t eq_pos = query_string.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(query_string.substr(0, eq_pos));
			value = UrlDecode(query_string.substr(eq_pos + 1));
			_get_params[key] = value;
		}
	}
}