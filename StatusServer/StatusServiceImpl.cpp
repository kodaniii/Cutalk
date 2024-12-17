#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "defs.h"
#include "RedisMgr.h"
#include <climits>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

//����ΨһUUID��token���ƣ�
std::string generate_unique_string() {
	// ����UUID����
	boost::uuids::uuid uuid = boost::uuids::random_generator()();

	std::string unique_string = to_string(uuid);

	return unique_string;
}

//����gRPC�ͻ������󣬻�ȡ��Ӧ���������
Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply){
	std::cout << "StatusServiceImpl::GetChatServer()" << std::endl;
	//������������ٵķ�����
	const auto& server = GetServer();
	reply->set_host(server.host);
	reply->set_port(server.port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(generate_unique_string());
	//�����û�uid��token�����뵽Redis
	insertToken(request->uid(), reply->token());

	std::cout << "Select " << server.name << "(" << server.host << ":" << server.port << ")" << std::endl;
	return Status::OK;
}

StatusServiceImpl::StatusServiceImpl(){
	std::cout << "StatusServiceImpl::StatusServiceImpl()" << std::endl;
	auto& gCfgMgr = ConfigMgr::init();
	auto server_list = gCfgMgr["ChatServers"]["name"];

	std::vector<std::string> words;

	std::stringstream ss(server_list);
	std::string word;

	while (std::getline(ss, word, ',')) {
		words.push_back(word);
	}

	for (auto& word : words) {
		std::cout << "_servers.push " << word << std::endl;
		if (gCfgMgr[word]["name"].empty()) {
			continue;
		}

		ChatServer server;
		server.port = gCfgMgr[word]["port"];
		server.host = gCfgMgr[word]["host"];
		server.name = gCfgMgr[word]["name"];
		_servers[server.name] = server;
	}
}

ChatServer StatusServiceImpl::GetServer() {
	std::cout << "StatusServiceImpl::GetServer()" << std::endl;
	std::lock_guard<std::mutex> lk(_server_mtx);

	std::cout << "*********************" << std::endl;
	std::cout << "ChatServers INFO:" << std::endl;
	for (auto _server : _servers) {
		std::cout << "[" << _server.first << "]" << std::endl;
		std::cout << "conn = " << _server.second.conn_count << std::endl;
		std::cout << "name = " << _server.second.name << std::endl;
		std::cout << "host = " << _server.second.host << std::endl;
		std::cout << "port = " << _server.second.port << std::endl;
	}
	std::cout << "*********************" << std::endl;

	auto minServer = _servers.begin()->second;
	auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);

	if (count_str.empty()) {
		//�����ڣ�˵����ӦChatServerû��HSet(LOGIN_COUNT, xxx)
		//Ĭ������������Ϊ���
		minServer.conn_count = INT_MAX;
	}
	else {
		minServer.conn_count = std::stoi(count_str);
	}

	//�ӵڶ���ChatServer��ʼ��������С������������
	for (auto& server : _servers) {
		if (server.second.name == minServer.name) {
			continue;
		}

		auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);
		if (count_str.empty()) {
			server.second.conn_count = INT_MAX;
		}
		else {
			server.second.conn_count = std::stoi(count_str);
		}

		if (server.second.conn_count < minServer.conn_count) {
			minServer = server.second;
		}
	}

	return minServer;
}

//����ͻ��˵ĵ�¼����
//ÿ�ε�¼��ȡһ���µ�token�����û�uid��token������Redis
Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply) {
	std::cout << "StatusServiceImpl::Login()" << std::endl;
	auto uid = request->uid();
	auto token = request->token();

	std::string uid_str = std::to_string(uid);
	std::string token_key = USER_TOKEN_PREFIX + uid_str;
	std::string token_value = "";
	bool succ = RedisMgr::GetInstance()->Get(token_key, token_value);
	std::cout << "RedisMgr::GetInstance()->Get() " << token_key << ": "
			<< token_value << ", ret " << succ << std::endl;
	//uid�Ҳ�����Ӧ��token
	if (!succ) {
		std::cout << "ERR RedisMgr::GetInstance()->Get() fail ret UidInvalid" << std::endl;
		reply->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}

	//token��ƥ��
	if (token_value != token) {
		std::cout << "ERR token_value != token ret TokenInvalid" << std::endl;
		reply->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
	}

	reply->set_error(ErrorCodes::Success);
	reply->set_uid(uid);
	reply->set_token(token);
	return Status::OK;
}

void StatusServiceImpl::insertToken(int uid, std::string token) {
	std::cout << "StatusServiceImpl::insertToken()" << std::endl;
	std::string uid_str = std::to_string(uid);
	std::string token_key = USER_TOKEN_PREFIX + uid_str;
	RedisMgr::GetInstance()->Set(token_key, token);
	std::cout << "RedisMgr::GetInstance()->Set() " << token_key << ": " << token << std::endl;
}

