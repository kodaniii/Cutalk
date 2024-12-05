#pragma once
#include "defs.h"
#include "Singleton.h"

class RedisPool {
public:
	RedisPool(size_t _size, const char* _host, int _port, const char* _pwd);
	~RedisPool();

	redisContext* GetConnection();

	void PushConnection(redisContext*);

	void Close();

private:
	std::atomic<bool> isStop;
	size_t poolSize;
	const char* host;
	int port;
	std::queue<redisContext*> conns;
	std::condition_variable cond;
	std::mutex mtx;
};

class RedisMgr : public Singleton<RedisMgr>
{
public:
	friend class Singleton<RedisMgr>;

	~RedisMgr();

	//bool Connect(const std::string& host, int port);
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

private:
	void Close();
	RedisMgr();

	//redisContext* conn;
	//redisReply* reply;
	std::unique_ptr<RedisPool> redis_pool;
};