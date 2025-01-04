#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include "CSession.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "RedisMgr.h"
#include "MysqlMgr.h"

ChatServiceImpl::ChatServiceImpl()
{

}

Status ChatServiceImpl::NotifyAddFriend(ServerContext *context, const AddFriendReq *request,
										AddFriendRsp *reply){
	std::cout << "ChatServiceImpl::NotifyAddFriend() recv handle" << std::endl;

	//验证对方session是否存在
	auto touid = request->touid();
	auto session = UserMgr::GetInstance()->GetSession(touid);

	Defer defer([request, reply]() {
		reply->set_error(ErrorCodes::Success);
		reply->set_applyuid(request->applyuid());
		reply->set_touid(request->touid());
	});

	if (session == nullptr) {
		return Status::OK;
	}

	//直接将相关信息通过TCP传输给对应Client，两个Client位于同一个ChatServer
	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["send_uid"] = request->applyuid();
	rtvalue["send_name"] = request->name();
	rtvalue["desc"] = request->desc();
	rtvalue["icon"] = request->icon();
	rtvalue["sex"] = request->sex();
	rtvalue["nick"] = request->nick();

	std::string return_str = rtvalue.toStyledString();

	session->Send(return_str, REQ_NOTIFY_ADD_FRIEND_REQ);
	return Status::OK;
}

Status ChatServiceImpl::NotifyAuthFriend(ServerContext* context,
	const AuthFriendReq* request, AuthFriendRsp* reply) {

	std::cout << "ChatServiceImpl::NotifyAuthFriend()" << std::endl;
	//查找用户是否在本服务器，找的是send好友申请的发送方
	auto send_uid = request->send_uid();
	auto recv_uid = request->recv_uid();
	auto session = UserMgr::GetInstance()->GetSession(send_uid);

	Defer defer([request, reply]() {
		reply->set_error(ErrorCodes::Success);
		reply->set_send_uid(request->send_uid());
		reply->set_recv_uid(request->recv_uid());
		});

	if (session == nullptr) {
		return Status::OK;
	}

	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["send_uid"] = request->send_uid();
	rtvalue["recv_uid"] = request->recv_uid();
	rtvalue["recv_name"] = request->recv_name();
	rtvalue["recv_nick"] = request->recv_nick();
	rtvalue["recv_icon"] = request->recv_icon();
	rtvalue["recv_sex"] = request->recv_sex();

	std::string return_str = rtvalue.toStyledString();

	session->Send(return_str, REQ_NOTIFY_AUTH_FRIEND_REQ);
	return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext* context,
	const TextChatMsgReq* request, TextChatMsgRsp* reply) {
	//TODO
	return Status::OK;
}


bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	//TODO 
	return true;
}