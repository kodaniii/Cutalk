#pragma once
#include "defs.h"
#include "Singleton.h"
#include "CSession.h"
#include "data.h"

typedef function<void(shared_ptr<CSession>, 
	const short& msg_type_id, const string& msg_data)> FunCallBack;

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
	void LoginHandler(shared_ptr<CSession> session, const short& msg_type_id, const string& msg_data);

	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);

	std::thread _work_thread;
	std::atomic<bool> _isStop;
	std::queue<shared_ptr<LogicNode>> _msg_que;
	std::mutex _mtx;
	std::condition_variable _cond;

	//msg_type_id, Func<void(shared_ptr<CSession>, msg_type_id, msg_data)>
	std::map<short, FunCallBack> _func_callbacks;
	//std::unordered_map<int, std::shared_ptr<UserInfo>> _users;
};


