#pragma once
#include "defs.h"
#include "MysqlDao.h"
#include "singleton.h"
#include "data.h"

class MysqlMgr : public Singleton<MysqlMgr>
{
	friend class Singleton<MysqlMgr>;
public:
	~MysqlMgr();
	
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUser(std::string name);

	bool AddFriendApply(int& from_id, int& to_id);

private:
	MysqlMgr();
	MysqlDao dao;
};
