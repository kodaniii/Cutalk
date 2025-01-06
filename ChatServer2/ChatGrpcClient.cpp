#include "ChatGrpcClient.h"
#include "RedisMgr.h"
#include "ConfigMgr.h"
#include "UserMgr.h"

#include "CSession.h"
#include "MysqlMgr.h"

ChatGrpcClient::ChatGrpcClient()
{
	auto& gCfgMgr = ConfigMgr::init();
	auto server_list = gCfgMgr["PeerServers"]["name"];	//ChatServer2, ChatServer3(FOR ChatServer1)

	std::vector<std::string> words;

	std::stringstream ss(server_list);
	std::string word;

	while (std::getline(ss, word, ',')) {
		words.push_back(word);
	}

	for (auto& word : words) {
		if (gCfgMgr[word]["name"].empty()) {
			continue;
		}

		//rpc_pool["ChatServer2"] = 
		rpc_pool[gCfgMgr[word]["name"]] =
			std::make_unique<ChatPool>(5, gCfgMgr[word]["host"], gCfgMgr[word]["RPCPort"]);
	}

}

ChatGrpcClient::~ChatGrpcClient()
{

}

/* 添加好友时，被添加方和添加方不在同一个ChatServer的情况，需要gRPC实现ChatServer之间的通信
 * 这个函数是发送，发AddFriendReq，回AddFriendRsp
 */
AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req)
{
	std::cout << "ChatGrpcClient::NotifyAddFriend() send and recv backpackage" << std::endl;
	//TODO, is ok
	AddFriendRsp rsp;
	Defer defer([&rsp, &req]() {
		rsp.set_applyuid(req.applyuid());
		rsp.set_touid(req.touid());
		});

	auto find_iter = rpc_pool.find(server_ip);
	if (find_iter == rpc_pool.end()) {
		std::cout << "find_iter == rpc_pool.end(), return..." << std::endl;
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->GetConnection();
	Status status = stub->NotifyAddFriend(&context, req, &rsp);	//ChatServiceImpl::NotifyAddFriend()
	Defer defercon([&stub, this, &pool]() {
		pool->PushConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::ChatFailed);
		return rsp;
	}

	rsp.set_error(ErrorCodes::Success);
	return rsp;

	//return AddFriendRsp();
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req) {
	AuthFriendRsp rsp;

	Defer defer([&rsp, &req]() {
		rsp.set_send_uid(req.send_uid());
		rsp.set_recv_uid(req.recv_uid());
		});

	auto find_iter = rpc_pool.find(server_ip);
	if (find_iter == rpc_pool.end()) {
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->GetConnection();
	Status status = stub->NotifyAuthFriend(&context, req, &rsp);
	Defer defercon([&stub, this, &pool]() {
		pool->PushConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::ChatFailed);
		return rsp;
	}

	rsp.set_error(ErrorCodes::Success);
	return rsp;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip,
	const TextChatMsgReq& req, const Json::Value& rtvalue) {
	TextChatMsgRsp rsp;
	rsp.set_error(ErrorCodes::Success);

	Defer defer([&rsp, &req]() {
		rsp.set_send_uid(req.send_uid());
		rsp.set_recv_uid(req.recv_uid());
		for (const auto& text_data : req.textmsgs()) {
			TextChatData* new_msg = rsp.add_textmsgs();
			new_msg->set_msg_id(text_data.msg_id());
			new_msg->set_msg_content(text_data.msg_content());
		}
	});

	auto find_iter = rpc_pool.find(server_ip);
	if (find_iter == rpc_pool.end()) {
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->GetConnection();
	Status status = stub->NotifyTextChatMsg(&context, req, &rsp);
	Defer defercon([&stub, this, &pool]() {
		pool->PushConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::ChatFailed);
		return rsp;
	}

	return rsp;
}