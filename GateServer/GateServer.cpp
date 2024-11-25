#include "defs.h"
#include "CServer.h"
#include "ConfigMgr.h"

void testRedis() {
	//redisContext* c = redisConnect("127.0.0.1", 6380);
}


int main() {
	ConfigMgr &gCfgMgr = ConfigMgr::init();
	
	std::string gateServerPort_str = gCfgMgr["GateServer"]["port"];
	unsigned short gateServerPort = atoi(gateServerPort_str.c_str());

	std::cout << "GateServer Port = " << gateServerPort << std::endl;

	try {
		unsigned short port = static_cast<unsigned short>(gateServerPort);
		net::io_context ioc{ 1 };
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const boost::system::error_code &err, int sig_id) {
			if (err) {
				return;
			}
			
			ioc.stop();
		});

		std::make_shared<CServer>(ioc, port)->start();
		std::cout << "Gate Server listen on port " << port << std::endl;
		ioc.run();
	}
	catch (const std::exception &e){
		std::cerr << "GateServer Error " << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return 0;
}