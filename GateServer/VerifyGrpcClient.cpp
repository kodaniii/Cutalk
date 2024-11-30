#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"

RPCPool::RPCPool(size_t _size, std::string _host, std::string _port)
	: poolSize(_size), host(_host), port(_port), isStop(false) {
	std::cout << "RPCPool::RPCPool(): poolSize, host, port={"
				<< _size << ", " << _host << ", " << _port << "}" << std::endl;
	for (size_t i = 0; i < poolSize; i++) {
		std::shared_ptr<Channel> chan = grpc::CreateChannel(host + ":" + port, //"127.0.0.1:50051",
									grpc::InsecureChannelCredentials());

		auto m = VerifyService::NewStub(chan);
		conns.push(std::move(m));
		//conns.push(VerifyService::NewStub(chan));
	}
}

std::unique_ptr<VerifyService::Stub> RPCPool::GetConnection() {
	std::cout << "RPCPool::GetConnection()" << std::endl;
	//std::lock_guard<std::mutex> lk(mtx);
	std::unique_lock<std::mutex> lk_gc(mtx);

	//wait结束条件：isStop为true || conns连接池队列非空 || 被cond.notify_one/all()唤醒
	cond.wait(lk_gc, [this]() {
		if (isStop) {
			return true;
		}
		return !conns.empty();
	});

	if (isStop) {
		return nullptr;
	}
	
	auto conn = std::move(conns.front());
	conns.pop();
	return conn;
}

void RPCPool::PushConnection(std::unique_ptr<VerifyService::Stub> conn) {
	std::cout << "RPCPool::PushConnection()" << std::endl;
	std::lock_guard<std::mutex> lk_pc(mtx);
	if (isStop) {
		return;
	}

	conns.push(std::move(conn));
	cond.notify_one();
}

RPCPool::~RPCPool() {
	//RAII
	std::lock_guard<std::mutex> lk_des(mtx);
	NotifyUsed();
	while (!conns.empty()) {
		conns.pop();
	}
}

void RPCPool::NotifyUsed() {
	isStop = true;
	cond.notify_all();
}

VerifyGrpcClient::VerifyGrpcClient() {
	ConfigMgr& gCfgMgr = ConfigMgr::init();
	
	std::string gateServerPort = gCfgMgr["VerifyServer"]["port"];
	std::string gateServerHost = gCfgMgr["VerifyServer"]["host"];

	//unsigned short gateServerPort = atoi(gateServerPort_str.c_str());
	//unsigned short gateServerHost = atoi(gateServerHost_str.c_str());

	std::cout << "GateServer Host = " << gateServerHost 
		<< ", Port = " << gateServerPort << std::endl;

	rpc_pool.reset(new RPCPool(5, gateServerHost, gateServerPort));
}