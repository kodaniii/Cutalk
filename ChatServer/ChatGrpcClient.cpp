#include "ChatGrpcClient.h"
#include "RedisMgr.h"
#include "ConfigMgr.h"
#include "UserMgr.h"

#include "CSession.h"
#include "MysqlMgr.h"

ChatGrpcClient::ChatGrpcClient()
{
	auto &gCfgMgr = ConfigMgr::init();
	auto server_list = gCfgMgr["PeerServers"]["name"];	//ChatServer2, ChatServer3(FOR ChatServer1)

	std::vector<std::string> words;

	std::stringstream ss(server_list);
	std::string word;

	while (std::getline(ss, word, ',')) {
		words.push_back(word);
	}

	for (auto &word : words) {
		if (gCfgMgr[word]["name"].empty()) {
			continue;
		}

		//rpc_pool["ChatServer2"] = 
		rpc_pool[gCfgMgr[word]["name"]] = 
			std::make_unique<ChatPool>(5, gCfgMgr[word]["host"], gCfgMgr[word]["port"]);
	}

}

ChatGrpcClient::~ChatGrpcClient()
{

}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req)
{
	//TODO
	return AddFriendRsp();
}


bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	//TODO
	return true;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req) {
	//TODO
	return AuthFriendRsp();
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip,
	const TextChatMsgReq& req, const Json::Value& rtvalue) {
	//TODO 
	return TextChatMsgRsp();
}