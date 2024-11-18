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

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
public:
	friend class Singleton<VerifyGrpcClient>;
	GetVerifyRsp GrpcVerifyCodeHandler(std::string email) {
		ClientContext context;
		GetVerifyReq request;
		GetVerifyRsp reply;
		request.set_email(email);
		
		Status status = _stub->GetVerifyCode(&context, request, &reply);
		std::cout << "GrpcVerifyCodeHandler status = " << (status.ok()? 0: 1) << std::endl;
		if (status.ok()) {
			return reply;
		}
		else {
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VerifyGrpcClient() {
		std::shared_ptr<Channel> chan = grpc::CreateChannel("127.0.0.1:50051", 
															grpc::InsecureChannelCredentials());
		_stub = VerifyService::NewStub(chan);
	}

	std::unique_ptr<VerifyService::Stub> _stub;
};


