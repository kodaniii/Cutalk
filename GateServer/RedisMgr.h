#pragma once
#include "defs.h"
#include "Singleton.h"

class RedisMgr: public Singleton<RedisMgr>
{
public:
	friend class Singleton<RedisMgr>;
	
	~RedisMgr();

	bool Connect(const std::string& host, int port);
	bool Get(const std::string& key, std::string& value);
	bool Set(const std::string& key, const std::string& value);
	bool Auth(const std::string& password);
	bool LPush(const std::string& key, const std::string& value);
	bool LPop(const std::string& key, std::string& value);
	bool RPush(const std::string& key, const std::string& value);
	bool RPop(const std::string& key, std::string& value);
	bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
	bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
	std::string HGet(const std::string& key, const std::string& hkey);
	bool Del(const std::string& key);
	bool ExistKey(const std::string& key);
	void Close();
private:
	RedisMgr();

	redisContext* conn;
	redisReply* reply;
};

