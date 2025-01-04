#pragma once
#include "defs.h"
#include "MysqlDao.h"
#include "singleton.h"
#include "data.h"
#include <string>

class MysqlMgr : public Singleton<MysqlMgr>
{
	friend class Singleton<MysqlMgr>;
public:
	~MysqlMgr();
	
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUser(std::string name);

	bool AddFriendApply(int& from_id, int& to_id, std::string& reason);
	bool GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit = 10);
	bool AuthFriendApply(const int& send_uid, const int& recv_uid, std::string recv_backname);


private:
	MysqlMgr();
	MysqlDao dao;
};
