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

bool MysqlMgr::AddFriendApply(int& from_id, int& to_id, std::string& reason) {
	return dao.AddFriendApply(from_id, to_id, reason);
}

bool MysqlMgr::GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList,
	int begin, int limit) {
	return dao.GetApplyList(touid, applyList, begin, limit);
}

bool MysqlMgr::AuthFriendApply(const int& send_uid, const int& recv_uid, std::string recv_backname) {
	return dao.AuthFriendApply(send_uid, recv_uid, recv_backname);
}

bool MysqlMgr::GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_info_list) {
	return dao.GetFriendList(self_id, user_info_list);
}