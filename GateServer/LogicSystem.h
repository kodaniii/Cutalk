#pragma once
#include "defs.h"
#include "Singleton.h"

//forward declaration
class HttpConnection;

typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;


class LogicSystem : public Singleton<LogicSystem>
{
public:
	friend class Singleton<LogicSystem>;
	
	~LogicSystem() {};
	
	bool HandleGet(std::string, std::shared_ptr<HttpConnection>);	//处理get请求
	void RegisterGet(std::string, HttpHandler);				//注册get请求
	bool HandlePost(std::string, std::shared_ptr<HttpConnection>);
	void RegisterPost(std::string, HttpHandler);

private:
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers;	//处理post请求的集合
	std::map<std::string, HttpHandler> _get_handlers;	//处理get请求的集合
};

