#include "RedisMgr.h"
#include "ConfigMgr.h"

RedisPool::RedisPool(size_t _size, const char* _host, int _port, const char* _pwd)
	: poolSize(_size), host(_host), port(_port), isStop(false) {
	std::cout << "RedisPool::RedisPool(poolSize, host, port) = {"
		<< _size << ", " << _host << ", " << _port
		<< "}" << std::endl;
	for (size_t i = 0; i < poolSize; ++i) {
		auto* rc = redisConnect(host, port);
		if (rc == nullptr || rc->err != 0) {
			if (rc != nullptr) {
				redisFree(rc);
			}
			continue;
		}

		auto reply = (redisReply*)redisCommand(rc, "AUTH %s", _pwd);
		if (reply->type == REDIS_REPLY_ERROR) {
			std::cout << "[RedisPool " << i << "] 认证失败" << std::endl;
			//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
			freeReplyObject(reply);
			continue;
		}

		//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
		freeReplyObject(reply);
		std::cout << "[RedisPool " << i << "] 认证成功" << std::endl;
		conns.push(rc);
	}
}

RedisPool::~RedisPool() {
	std::lock_guard<std::mutex> lk(mtx);
	while (!conns.empty()) {
		conns.pop();
	}
}

redisContext* RedisPool::getConnection() {
	std::unique_lock<std::mutex> lk(mtx);
	cond.wait(lk, [this]() {
		if (isStop) {
			return true;
		}
		return !conns.empty();
	});

	//如果停止则直接返回空指针
	if (isStop) {
		return nullptr;
	}

	auto *rc = conns.front();
	conns.pop();
	return rc;
}

void RedisPool::PushConnection(redisContext* rc) {
	std::lock_guard<std::mutex> lk(mtx);
	if (isStop) {
		return;
	}
	conns.push(rc);
	cond.notify_one();
}

void RedisPool::Close() {
	isStop = true;
	cond.notify_all();
}

RedisMgr::RedisMgr() {
	ConfigMgr& gCfgMgr = ConfigMgr::init();

	std::string redisMgrhost = gCfgMgr["Redis"]["host"];
	std::string redisMgrport = gCfgMgr["Redis"]["port"];
	std::string redisMgrpswd = gCfgMgr["Redis"]["pswd"];

	redis_pool.reset(new RedisPool(5, redisMgrhost.c_str(), 
				atoi(redisMgrport.c_str()), redisMgrpswd.c_str()));
}

RedisMgr::~RedisMgr() {
	this->Close();
}

/*
bool RedisMgr::Connect(const std::string& host, int port) {
	if (this->conn == nullptr) {
		return false;
	}
	if (this->conn != NULL && this->conn->err) {
		std::cout << "RedisMgr::Connect err " << this->conn->errstr << std::endl;
		return false;
	}
	
	return true;
}*/

void RedisMgr::Close() {
	//redisFree(this->conn);
	this->redis_pool->Close();
}

bool RedisMgr::Get(const std::string& key, std::string& value){
	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}

	//reply = (redisReply*)redisCommand(conn, "GET %s", key.c_str());
	auto reply = (redisReply*)redisCommand(conn, "GET %s", key.c_str());
	if (reply == NULL) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] GET " << key << " failed" << std::endl;
		return false;
	}

	if (reply->type != REDIS_REPLY_STRING) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] GET " << key << " failed" << std::endl;
		return false;
	}

	value = reply->str;
	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Succeed to execute command GET " << key << std::endl;
	return true;
}

bool RedisMgr::Set(const std::string& key, const std::string& value) {
	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}

	auto reply = (redisReply*)redisCommand(conn, "SET %s %s", key.c_str(), value.c_str());
	if (reply == NULL){
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] SET " << key << "  " << value << " failed" << std::endl;
		return false;
	}

	if (!(reply->type == REDIS_REPLY_STATUS
		&& (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))){
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] SET " << key << "  " << value << " failed" << std::endl;
		return false;
	}

	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Succeed to execute command SET " << key << " " << value << std::endl;
	return true;
}

bool RedisMgr::Auth(const std::string& password){
	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}

	auto reply = (redisReply*)redisCommand(conn, "AUTH %s", password.c_str());
	if (reply->type == REDIS_REPLY_ERROR) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "认证失败" << std::endl;
		return false;
	}
	else {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "认证成功" << std::endl;
		return true;
	}
}

bool RedisMgr::LPush(const std::string& key, const std::string& value){
	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}

	auto reply = (redisReply*)redisCommand(conn, "LPUSH %s %s", key.c_str(), value.c_str());
	if (reply == NULL){
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] LPUSH " << key << "  " << value << " failed" << std::endl;
		return false;
	}

	if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] LPUSH " << key << "  " << value << " failed" << std::endl;
		return false;
	}

	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Succeed to execute command LPUSH " << key << " " << value << std::endl;
	return true;
}

bool RedisMgr::LPop(const std::string& key, std::string& value) {
	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}

	auto reply = (redisReply*)redisCommand(conn, "LPOP %s ", key.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] LPOP " << key << "  " << value << " failed" << std::endl;
		return false;
	}

	value = reply->str;
	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Succeed to execute command LPOP " << key << std::endl;
	return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value) {
	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}

	auto reply = (redisReply*)redisCommand(conn, "RPUSH %s %s", key.c_str(), value.c_str());
	if (reply == NULL){
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] RPUSH " << key << "  " << value << " failed" << std::endl;
		return false;
	}

	if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] RPUSH " << key << "  " << value << " failed" << std::endl;
		return false;
	}

	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Succeed to execute command RPUSH " << key << " " << value << std::endl;
	return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value) {
	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}

	auto reply = (redisReply*)redisCommand(conn, "RPOP %s ", key.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] RPOP " << key << " failed" << std::endl;
		return false;
	}
	
	value = reply->str;
	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Succeed to execute command RPOP " << key << std::endl;
	return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value) {
	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}
	
	auto reply = (redisReply*)redisCommand(conn, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] HSet " << key << "  " << hkey << "  " << value << " failed" << std::endl;
		return false;
	}
	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Succeed to execute command HSet " << key << "  " << hkey << "  " << value << std::endl;
	return true;
}


bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen){
	const char* argv[4];
	size_t argvlen[4];
	argv[0] = "HSET";
	argvlen[0] = 4;
	argv[1] = key;
	argvlen[1] = strlen(key);
	argv[2] = hkey;
	argvlen[2] = strlen(hkey);
	argv[3] = hvalue;
	argvlen[3] = hvaluelen;

	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}

	auto reply = (redisReply*)redisCommandArgv(conn, 4, argv, argvlen);
	if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] HSet " << key << "  " << hkey << "  " << hvalue << " failed" << std::endl;
		return false;
	}

	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Succeed to execute command HSet " << key << "  " << hkey << "  " << hvalue << std::endl;
	return true;
}

std::string RedisMgr::HGet(const std::string& key, const std::string& hkey){
	const char* argv[3];
	size_t argvlen[3];
	argv[0] = "HGET";
	argvlen[0] = 4;
	argv[1] = key.c_str();
	argvlen[1] = key.length();
	argv[2] = hkey.c_str();
	argvlen[2] = hkey.length();

	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return "";
	}

	auto reply = (redisReply*)redisCommandArgv(conn, 3, argv, argvlen);
	if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] HGet " << key << " " << hkey << " failed" << std::endl;
		return "";
	}

	std::string value = reply->str;
	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Succeed to execute command HGet " << key << " " << hkey << std::endl;
	return value;
}

bool RedisMgr::Del(const std::string& key){
	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}

	auto reply = (redisReply*)redisCommand(conn, "DEL %s", key.c_str());
	if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] Del " << key << "failed" << std::endl;
		return false;
	}

	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Succeed to execute command Del " << key << std::endl;
	return true;
}

bool RedisMgr::ExistKey(const std::string& key)
{
	//redis pool
	auto conn = redis_pool->getConnection();
	if (conn == nullptr) {
		return false;
	}

	auto reply = (redisReply*)redisCommand(conn, "exists %s", key.c_str());
	if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {
		freeReplyObject(reply);
		this->redis_pool->PushConnection(conn);
		std::cout << "[Redis] Not Found Key " << key << std::endl;
		return false;
	}

	freeReplyObject(reply);
	this->redis_pool->PushConnection(conn);
	std::cout << "[Redis] Found Key " << key << " exist" << std::endl;
	return true;
}