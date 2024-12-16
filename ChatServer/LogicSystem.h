#pragma once
#include "defs.h"
#include "Singleton.h"
#include "CSession.h"

typedef function<void(shared_ptr<CSession>, 
	const short& msg_id, const string& msg_data)> FunCallBack;

class LogicSystem : public Singleton<LogicSystem>
{
public:
	friend class Singleton<LogicSystem>;
	
	~LogicSystem();
	void PostMsgToQue(std::shared_ptr<LogicNode> msg);

private:
	LogicSystem();

	void DealMsg();
	void RegisterCallBacks();

	std::thread _work_thread;
	std::atomic<bool> _isStop;
	std::queue<shared_ptr<LogicNode>> _msg_que;
	std::mutex _mtx;
	std::condition_variable _cond;

	//msg_id, Func<void(shared_ptr<CSession>, msg_id, msg_data)>
	std::map<short, FunCallBack> _func_callbacks;
};


