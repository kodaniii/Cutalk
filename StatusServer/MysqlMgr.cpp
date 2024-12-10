#include "MysqlMgr.h"

MysqlMgr::MysqlMgr() {
	std::cout << "MysqlMgr::MysqlMgr()" << std::endl;
}

MysqlMgr::~MysqlMgr() {

}

int MysqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& pswd)
{
	std::cout << "MysqlMgr::RegUser()" << std::endl;
	return dao.RegUser(name, email, pswd);
}

int MysqlMgr::CheckResetIsVaild(const std::string& name, const std::string& email) {
	std::cout << "MysqlMgr::CheckResetIsVaild()" << std::endl;
	return dao.CheckResetIsVaild(name, email);
}

bool MysqlMgr::UpdateUserAndPswd(const std::string& name, const std::string& pswd, const std::string& email) {
	std::cout << "MysqlMgr::UpdateUserAndPswd()" << std::endl;
	return dao.UpdateUserAndPswd(name, pswd, email);
}

bool MysqlMgr::LoginCheckPswd(const std::string& name_or_email, const std::string& pswd, UserInfo& userInfo) {
	std::cout << "MysqlMgr::LoginCheckPswd()" << std::endl;
	return dao.LoginCheckPswd(name_or_email, pswd, userInfo);
}