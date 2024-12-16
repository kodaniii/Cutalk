// ChatServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "LogicSystem.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOServicePool.h"
#include "CServer.h"
#include "ConfigMgr.h"
using namespace std;

int main(){
	try {
		auto &gCfgMgr = ConfigMgr::init();
		auto ioc_pool = AsioIOServicePool::GetInstance();
		boost::asio::io_context ioc;
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		//停止信号
		//auto auto <- const boost::system::error_code& error, int signal_number
		signals.async_wait([&ioc, ioc_pool](auto, auto) {
			ioc.stop();
			ioc_pool->Stop();
		});
		auto port_str = gCfgMgr["SelfServer"]["port"];
		cout << "ChatServer listening on port " << port_str << endl;
		CServer s(ioc, atoi(port_str.c_str()));
		ioc.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}
}

