#pragma once
#include "defs.h"
#include "Singleton.h"
#include "ConfigMgr.h"
#include <grpcpp/grpcpp.h> 
#include "message.grpc.pb.h"
#include "message.pb.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginRsp;
using message::LoginReq;
using message::StatusService;

class StatusPool {
public:
	StatusPool(size_t _size, std::string _host, std::string _port)
		: poolSize(_size), host(_host), port(_port), isStop(false) {
		std::cout << "StatusPool::StatusPool(): poolSize, host, port={"
			<< _size << ", " << _host << ", " << _port << "}" << std::endl;
		for (size_t i = 0; i < poolSize; ++i) {
			std::shared_ptr<Channel> chan = grpc::CreateChannel(host + ":" + port,
				grpc::InsecureChannelCredentials());

			conns.push(StatusService::NewStub(chan));
		}
	}

	~StatusPool() {
		std::lock_guard<std::mutex> lk(mtx);
		NotifyUsed();
		while (!conns.empty()) {
			conns.pop();
		}
	}

	std::unique_ptr<StatusService::Stub> GetConnection() {
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

	void PushConnection(std::unique_ptr<StatusService::Stub> conn) {
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
	std::queue<std::unique_ptr<StatusService::Stub>> conns;
	std::condition_variable cond;
	std::mutex mtx;
};

class StatusGrpcClient :public Singleton<StatusGrpcClient>
{
	friend class Singleton<StatusGrpcClient>;
public:
	~StatusGrpcClient() { }

	LoginRsp Login(int uid, std::string token);

private:
	StatusGrpcClient();
	std::unique_ptr<StatusPool> rpc_pool;

};



