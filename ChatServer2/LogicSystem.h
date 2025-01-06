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
	void SearchInfo(std::shared_ptr<CSession> session, const short& msg_type_id, const string& msg_data);
	void AddFriendApply(std::shared_ptr<CSession> session, const short& msg_type_id, const string& msg_data);
	void AuthFriendApply(std::shared_ptr<CSession> session, const short& msg_type_id, const string& msg_data);
	void DealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_type_id, const string& msg_data);


	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	bool isPureDigit(const std::string& str);
	void GetUserByUid(std::string uid_str, Json::Value& rtvalue);
	void GetUserByName(std::string name, Json::Value& rtvalue);
	bool GetFriendApplyInfo(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list);
	bool GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list);


	std::thread _work_thread;
	std::atomic<bool> _isStop;
	std::queue<shared_ptr<LogicNode>> _msg_que;
	std::mutex _mtx;
	std::condition_variable _cond;

	//msg_type_id, Func<void(shared_ptr<CSession>, msg_type_id, msg_data)>
	std::map<short, FunCallBack> _func_callbacks;
	//std::unordered_map<int, std::shared_ptr<UserInfo>> _users;
};


