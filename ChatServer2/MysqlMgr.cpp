#include "MysqlMgr.h"

MysqlMgr::MysqlMgr() {
	std::cout << "MysqlMgr::MysqlMgr()" << std::endl;
}

MysqlMgr::~MysqlMgr() {

}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(int uid)
{
	return dao.GetUser(uid);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(std::string name)
{
	return dao.GetUser(name);
}

bool MysqlMgr::AddFriendApply(int& from_id, int& to_id) {
	return dao.AddFriendApply(from_id, to_id);
}