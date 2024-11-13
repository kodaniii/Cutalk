#include "LogicSystem.h"
#include "HttpConnection.h"

LogicSystem::LogicSystem() {
	RegisterGet("/get_test", [](std::shared_ptr<HttpConnection> conn) {
		beast::ostream(conn->_response.body()) << "receive get_test req" << std::endl;
		
		int i = 0;
		for (auto& elem : conn->_get_params) {
			i++;
			beast::ostream(conn->_response.body()) << "param " << i 
				<< ", key is " << elem.first
				<< ", value is " << elem.second << std::endl;
		}
	});
}

void LogicSystem::RegisterGet(std::string url, HttpHandler handler) {
	_get_handlers.insert(make_pair(url, handler));
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> conn) {
	//not found
	if (_get_handlers.find(path) == _get_handlers.end()) {
		return false;
	}

	_get_handlers[path](conn);
	
	return true;
}

