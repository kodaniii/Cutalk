#include "RedisMgr.h"

RedisMgr::RedisMgr() {
	
}

RedisMgr::~RedisMgr() {

}

bool RedisMgr::Connect(const std::string& host, int port) {
	this->conn = redisConnect(host.c_str(), port);
	if (this->conn == nullptr) {
		return false;
	}
	if (this->conn != NULL && this->conn->err) {
		std::cout << "RedisMgr::Connect err " << this->conn->errstr << std::endl;
		return false;
	}
	
	return true;
}

void RedisMgr::Close()
{
	redisFree(this->conn);
}

bool RedisMgr::Get(const std::string& key, std::string& value){
	this->reply = (redisReply*)redisCommand(conn, "GET %s", key.c_str());
	if (this->reply == NULL) {
		std::cout << "[Redis] GET " << key << " failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}

	if (this->reply->type != REDIS_REPLY_STRING) {
		std::cout << "[Redis] GET " << key << " failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}

	value = this->reply->str;
	freeReplyObject(this->reply);

	std::cout << "[Redis] Succeed to execute command GET " << key << std::endl;
	return true;
}

bool RedisMgr::Set(const std::string& key, const std::string& value) {
	this->reply = (redisReply*)redisCommand(this->conn, "SET %s %s", key.c_str(), value.c_str());
	if (this->reply == NULL){
		std::cout << "[Redis] SET " << key << "  " << value << " failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}

	if (!(this->reply->type == REDIS_REPLY_STATUS
		&& (strcmp(this->reply->str, "OK") == 0 || strcmp(this->reply->str, "ok") == 0))){
		std::cout << "[Redis] SET " << key << "  " << value << " failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}

	freeReplyObject(this->reply);
	std::cout << "[Redis] Succeed to execute command SET " << key << " " << value << std::endl;
	return true;
}

bool RedisMgr::Auth(const std::string& password){
	this->reply = (redisReply*)redisCommand(this->conn, "AUTH %s", password.c_str());
	if (this->reply->type == REDIS_REPLY_ERROR) {
		std::cout << "认证失败" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}
	else {
		freeReplyObject(this->reply);
		std::cout << "认证成功" << std::endl;
		return true;
	}
}

bool RedisMgr::LPush(const std::string& key, const std::string& value){
	this->reply = (redisReply*)redisCommand(this->conn, "LPUSH %s %s", key.c_str(), value.c_str());
	if (this->reply == NULL){
		std::cout << "[Redis] LPUSH " << key << "  " << value << " failed" << std::endl; 
		freeReplyObject(this->reply);
		return false;
	}

	if (this->reply->type != REDIS_REPLY_INTEGER || this->reply->integer <= 0) {
		std::cout << "[Redis] LPUSH " << key << "  " << value << " failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}

	std::cout << "[Redis] Succeed to execute command LPUSH " << key << " " << value << std::endl;
	freeReplyObject(this->reply);
	return true;
}

bool RedisMgr::LPop(const std::string& key, std::string& value) {
	this->reply = (redisReply*)redisCommand(this->conn, "LPOP %s ", key.c_str());
	if (this->reply == nullptr || this->reply->type == REDIS_REPLY_NIL) {
		std::cout << "[Redis] LPOP " << key << "  " << value << " failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}
	value = reply->str;
	std::cout << "[Redis] Succeed to execute command LPOP " << key << std::endl;
	freeReplyObject(this->reply);
	return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value) {
	this->reply = (redisReply*)redisCommand(this->conn, "RPUSH %s %s", key.c_str(), value.c_str());
	if (this->reply == NULL){
		std::cout << "[Redis] RPUSH " << key << "  " << value << " failed" << std::endl; 
		freeReplyObject(this->reply);
		return false;
	}

	if (this->reply->type != REDIS_REPLY_INTEGER || this->reply->integer <= 0) {
		std::cout << "[Redis] RPUSH " << key << "  " << value << " failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}

	std::cout << "[Redis] Succeed to execute command RPUSH " << key << " " <<  value << std::endl;
	freeReplyObject(this->reply);
	return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value) {
	this->reply = (redisReply*)redisCommand(this->conn, "RPOP %s ", key.c_str());
	if (this->reply == nullptr || this->reply->type == REDIS_REPLY_NIL) {
		std::cout << "[Redis] RPOP " << key << " failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}
	value = reply->str;
	
	std::cout << "[Redis] Succeed to execute command RPOP " << key << std::endl;
	freeReplyObject(this->reply);
	return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value) {
	this->reply = (redisReply*)redisCommand(this->conn, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	if (this->reply == nullptr || this->reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "[Redis] HSet " << key << "  " << hkey << "  " << value << " failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}
	std::cout << "[Redis] Succeed to execute command HSet " << key << "  " << hkey << "  " << value << std::endl;
	freeReplyObject(this->reply);
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
	this->reply = (redisReply*)redisCommandArgv(this->conn, 4, argv, argvlen);
	if (this->reply == nullptr || this->reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "[Redis] HSet " << key << "  " << hkey << "  " << hvalue << " failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}
	std::cout << "[Redis] Succeed to execute command HSet " << key << "  " << hkey << "  " << hvalue << std::endl;
	freeReplyObject(this->reply);
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
	this->reply = (redisReply*)redisCommandArgv(this->conn, 3, argv, argvlen);
	if (this->reply == nullptr || this->reply->type == REDIS_REPLY_NIL) {
		freeReplyObject(this->reply);
		std::cout << "[Redis] HGet " << key << " " << hkey << " failed" << std::endl;
		return "";
	}

	std::string value = this->reply->str;
	freeReplyObject(this->reply);
	std::cout << "[Redis] Succeed to execute command HGet " << key << " " << hkey << std::endl;
	return value;
}

bool RedisMgr::Del(const std::string& key){
	this->reply = (redisReply*)redisCommand(this->conn, "DEL %s", key.c_str());
	if (this->reply == nullptr || this->reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "[Redis] Del " << key << "failed" << std::endl;
		freeReplyObject(this->reply);
		return false;
	}
	std::cout << "[Redis] Succeed to execute command Del " << key << std::endl;
	freeReplyObject(this->reply);
	return true;
}

bool RedisMgr::ExistKey(const std::string& key)
{
	this->reply = (redisReply*)redisCommand(this->conn, "exists %s", key.c_str());
	if (this->reply == nullptr || this->reply->type != REDIS_REPLY_INTEGER || this->reply->integer == 0) {
		std::cout << "[Redis] Not Found Key " << key << std::endl;
		freeReplyObject(this->reply);
		return false;
	}
	std::cout << "[Redis] Found Key " << key << " exist" << std::endl;
	freeReplyObject(this->reply);
	return true;
}