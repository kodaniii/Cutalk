#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "defs.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class RPCPool {
public:
	RPCPool(size_t _size, std::string _host, std::string port);
	~RPCPool();
	void NotifyUsed();
	
	std::unique_ptr<VerifyService::Stub> GetConnection();
	void PushConnection(std::unique_ptr<VerifyService::Stub>);

private:
	std::atomic<bool> isStop;
	size_t poolSize;
	std::string host;
	std::string port;
	std::queue<std::unique_ptr<VerifyService::Stub>> conns;
	std::condition_variable cond;
	std::mutex mtx;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
public:
	friend class Singleton<VerifyGrpcClient>;
	GetVerifyRsp GrpcVerifyCodeHandler(std::string email) {
		ClientContext context;
		GetVerifyReq request;
		GetVerifyRsp reply;
		request.set_email(email);
		
		//grpc pool
		auto _stub = rpc_pool->GetConnection();

		Status status = _stub->GetVerifyCode(&context, request, &reply);
		std::cout << "VerifyGrpcClient::GrpcVerifyCodeHandler status = " << (status.ok()? "true" : "false") << std::endl;
		if (status.ok()) {
			//正常情况conn回收至grpc pool
			rpc_pool->PushConnection(std::move(_stub));
			return reply;
		}
		else {
			//异常情况conn回收至grpc pool
			rpc_pool->PushConnection(std::move(_stub));
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VerifyGrpcClient();
		/*
		std::shared_ptr<Channel> chan = grpc::CreateChannel("127.0.0.1:50051",
							grpc::InsecureChannelCredentials());
		_stub = VerifyService::NewStub(chan);*/

	//std::unique_ptr<VerifyService::Stub> _stub;
	std::unique_ptr<RPCPool> rpc_pool;
};


