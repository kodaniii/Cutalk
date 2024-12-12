#include "StatusGrpcClient.h"

StatusGrpcClient::StatusGrpcClient()
{
	auto& gCfgMgr = ConfigMgr::init();
	std::string host = gCfgMgr["StatusServer"]["host"];
	std::string port = gCfgMgr["StatusServer"]["port"];
	rpc_pool.reset(new StatusPool(5, host, port));
}

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
{
	ClientContext context;
	GetChatServerRsp reply;
	GetChatServerReq request;
	request.set_uid(uid);
	auto stub = rpc_pool->GetConnection();
	Status stat = stub->GetChatServer(&context, request, &reply);

	Defer defer([&stub, this]() {
		rpc_pool->PushConnection(std::move(stub));
	});

	if (stat.ok()) {
		return reply;
	}
	else {
		reply.set_error(ErrorCodes::StatusFailed);
		return reply;
	}
}