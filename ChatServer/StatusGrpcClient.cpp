#include "StatusGrpcClient.h"

StatusGrpcClient::StatusGrpcClient()
{
	auto& gCfgMgr = ConfigMgr::init();
	std::string host = gCfgMgr["StatusServer"]["host"];
	std::string port = gCfgMgr["StatusServer"]["port"];
	rpc_pool.reset(new StatusPool(5, host, port));
}

LoginRsp StatusGrpcClient::Login(int uid, std::string token)
{
	ClientContext context;
	LoginRsp reply;
	LoginReq request;
	request.set_uid(uid);
	request.set_token(token);

	auto stub = rpc_pool->GetConnection();
	Status status = stub->Login(&context, request, &reply);
	Defer defer([&stub, this]() {
		rpc_pool->PushConnection(std::move(stub));
	});
	if (status.ok()) {
		return reply;
	}
	else {
		reply.set_error(ErrorCodes::StatusFailed);
		return reply;
	}
}