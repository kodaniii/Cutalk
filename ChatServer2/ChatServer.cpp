// ChatServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "LogicSystem.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOServicePool.h"
#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "ChatServiceImpl.h"

using namespace std;

int main(){
	auto& gCfgMgr = ConfigMgr::init();
	auto server_name = gCfgMgr["SelfServer"]["name"];

	try {
		auto ioc_pool = AsioIOServicePool::GetInstance();
		/*初始化服务器登录数量到Redis，为了后面StatusServer的负载均衡策略*/
		RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, "0");

		/*监听gRPC，开放本地port*/
		std::string server_address(gCfgMgr["SelfServer"]["host"] + ":" + gCfgMgr["SelfServer"]["RPCPort"]);
		ChatServiceImpl service;
		grpc::ServerBuilder builder;
		//监听端口和添加服务
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		//构建并启动gRPC服务器
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
		std::cout << "RPC Server listening on " << server_address << std::endl;

		//单独启动一个线程处理gRPC服务
		std::thread grpc_server_thread([&server]() {
			server->Wait();
		});


		/*监听系统信号*/
		boost::asio::io_context ioc;
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		//停止信号
		//auto auto <- const boost::system::error_code& error, int signal_number
		signals.async_wait([&ioc, ioc_pool, &server](auto, auto) {
			ioc.stop();
			ioc_pool->Stop();
			server->Shutdown();
		});
		auto port_str = gCfgMgr["SelfServer"]["port"];
		cout << "ChatServer listening on port " << port_str << endl;
		CServer s(ioc, atoi(port_str.c_str()));
		ioc.run();


		RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);
		RedisMgr::GetInstance()->Close();
		grpc_server_thread.join();

	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}
}

