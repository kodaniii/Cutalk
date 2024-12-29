#pragma once
#include "defs.h"
#include "Singleton.h"
#include "ConfigMgr.h"
#include <grpcpp/grpcpp.h> 
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <queue>
#include "data.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>


using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::GetChatServerRsp;
using message::LoginRsp;
using message::LoginReq;
using message::ChatService;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;


class ChatPool {
public:
	ChatPool(size_t _size, std::string _host, std::string _port)
		: poolSize(_size), host(_host), port(_port), isStop(false) {
		std::cout << "ChatPool::ChatPool(): poolSize, host, port={"
			<< _size << ", " << _host << ", " << _port << "}" << std::endl;
		for (size_t i = 0; i < poolSize; ++i) {
			std::shared_ptr<Channel> chan = grpc::CreateChannel(host + ":" + port,
				grpc::InsecureChannelCredentials());

			conns.push(ChatService::NewStub(chan));
		}
	}

	~ChatPool() {
		std::cout << "ChatPool::~ChatPool()" << std::endl;

		std::lock_guard<std::mutex> lk(mtx);
		NotifyUsed();
		while (!conns.empty()) {
			conns.pop();
		}

	}

	std::unique_ptr<ChatService::Stub> GetConnection() {
		std::unique_lock<std::mutex> lk(mtx);
		cond.wait(lk, [this] {
			if (isStop) {
				return true;
			}
			return !conns.empty();
			});
		if (isStop) {
			return  nullptr;
		}
		auto conn = std::move(conns.front());
		conns.pop();
		return conn;
	}

	void PushConnection(std::unique_ptr<ChatService::Stub> conn) {
		std::lock_guard<std::mutex> lk(mtx);
		if (isStop) {
			return;
		}
		conns.push(std::move(conn));
		cond.notify_one();
	}

	void NotifyUsed() {
		isStop = true;
		cond.notify_all();
	}

private:
	std::atomic<bool> isStop;
	size_t poolSize;
	std::string host;
	std::string port;
	std::queue<std::unique_ptr<ChatService::Stub>> conns;
	std::condition_variable cond;
	std::mutex mtx;
};

class ChatGrpcClient :public Singleton<ChatGrpcClient>
{
	friend class Singleton<ChatGrpcClient>;
public:
	~ChatGrpcClient();

	AddFriendRsp NotifyAddFriend(std::string server_ip, const AddFriendReq& req);
	AuthFriendRsp NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req);
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	TextChatMsgRsp NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue);

private:
	ChatGrpcClient();
	/* 维护多个已连接的聊天服务器的gRPC连接池
	 * 对于每个不同的服务器(key)，都有一个ChatServerGrpcPool智能指针
	 */
	std::unordered_map<std::string, std::unique_ptr<ChatPool>> rpc_pool;
};



