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
	
	bool HandleGet(std::string, std::shared_ptr<HttpConnection>);	//����get����
	void RegisterGet(std::string, HttpHandler);				//ע��get����

private:
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers;	//����post����ļ���
	std::map<std::string, HttpHandler> _get_handlers;	//����get����ļ���
};

