#include "LogicSystem.h"
#include "HttpConnection.h"

LogicSystem::LogicSystem() {
	//Get测试
	RegisterGet("/get_test", [](std::shared_ptr<HttpConnection> conn) {
		std::cout << "/get_test get handler" << std::endl;
		beast::ostream(conn->_response.body()) << "receive get_test req" << std::endl;
		
		int i = 0;
		for (auto& elem : conn->_get_params) {
			i++;
			beast::ostream(conn->_response.body()) << "param " << i 
				<< ", key is " << elem.first
				<< ", value is " << elem.second << std::endl;
		}
	});

	//收取注册邮箱，发送验证码
	RegisterPost("/get_verifycode", [](std::shared_ptr<HttpConnection> conn) {
		std::cout << "/get_verifycode post handler" << std::endl;
		auto body_str = boost::beast::buffers_to_string(conn->_request.body().data());
		std::cout << "_request.body().data() = " << body_str << std::endl;
		
		conn->_response.set(http::field::content_type, "text/json");

		Json::Value root;		//dst
		Json::Reader reader;
		Json::Value src_root;
		
		bool parse_succ = reader.parse(body_str, src_root);	//src_root <- (Json)body_str;
		//解析失败
		if (!parse_succ) {
			std::cout << "Failed to parse JSON data" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string json_str = root.toStyledString();
			beast::ostream(conn->_response.body()) << json_str;
			return;
		}

		//没有key
		if (!src_root.isMember("email")) {
			std::cout << "Failed to parse JSON data" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string json_str = root.toStyledString();
			beast::ostream(conn->_response.body()) << json_str;
			return;
		}

		auto email = src_root["email"].asString();
		std::cout << "src_root[\"email\"] = " << email << std::endl;
		root["error"] = ErrorCodes::Success;
		root["email"] = email;
		std::string json_str = root.toStyledString();
		beast::ostream(conn->_response.body()) << json_str;
	});
}

void LogicSystem::RegisterGet(std::string url, HttpHandler handler) {
	_get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::RegisterPost(std::string url, HttpHandler handler) {
	_post_handlers.insert(make_pair(url, handler));
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> conn) {
	//not found
	if (_get_handlers.find(path) == _get_handlers.end()) {
		return false;
	}

	_get_handlers[path](conn);
	
	return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> conn) {
	if (_post_handlers.find(path) == _post_handlers.end()) {
		return false;
	}

	_post_handlers[path](conn);

	return true;
}