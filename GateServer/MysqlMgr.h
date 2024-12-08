#pragma once
#include "defs.h"
#include "MysqlDao.h"
#include "singleton.h"

class MysqlMgr : public Singleton<MysqlMgr>
{
	friend class Singleton<MysqlMgr>;
public:
	~MysqlMgr();
	int RegUser(const std::string& name, const std::string& email, const std::string& pswd);
	int CheckResetIsVaild(const std::string& name, const std::string& email);
	bool UpdateUserAndPswd(const std::string& name, const std::string& pswd, const std::string& email);

private:
	MysqlMgr();
	MysqlDao dao;
};
