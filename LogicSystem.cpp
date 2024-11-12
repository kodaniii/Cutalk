#include "LogicSystem.h"
#include "HttpConnection.h"

LogicSystem::LogicSystem() {
	RegisterGet("/get_test", [](std::shared_ptr<HttpConnection> conn) {
		beast::ostream(conn->_response.body()) << "receive get_test req";
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