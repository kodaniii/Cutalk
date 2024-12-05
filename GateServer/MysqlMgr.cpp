#include "MysqlMgr.h"


MysqlMgr::~MysqlMgr() {

}

int MysqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& pswd)
{
	std::cout << "MysqlMgr::RegUser()" << std::endl;
	return dao.RegUser(name, email, pswd);
}

MysqlMgr::MysqlMgr() {
	std::cout << "MysqlMgr::MysqlMgr()" << std::endl;
}