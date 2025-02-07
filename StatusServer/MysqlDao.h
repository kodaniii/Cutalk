#pragma once
#include "defs.h"
#include <thread>

/*
	维护sql_conn和last_op_time
	last_op_time:	该连接最后一次操作的时间
	用于放入queue<std::unique_ptr<MysqlConnection>> conns
*/
class MysqlConnection {
public:
	MysqlConnection(sql::Connection* _conn, int64_t last_time) 
		: sql_conn(_conn), last_op_time(last_time){	}

	std::unique_ptr<sql::Connection> sql_conn;
	int64_t last_op_time;
};

/*
	维护sql_conn的user、pswd、url、schema等信息
	MysqlConnection池
*/
class MysqlPool {
public:
	MysqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);
	~MysqlPool();

	std::unique_ptr<MysqlConnection> GetConnection();
	void PushConnection(std::unique_ptr<MysqlConnection> conn);
	void CheckConnection();
	void Close();

private:
	std::string url;
	std::string user;
	std::string pswd;
	std::string schema;

	size_t poolSize;
	std::atomic<bool> isStop;
	//每一个conn都是一个MysqlConnection
	//MysqlConnection::MysqlConnection(sql_conn, last_time)
	//conn->sql_conn
	std::queue<std::unique_ptr<MysqlConnection>> conns;
	std::mutex mtx;
	std::condition_variable cond;

	std::thread detach_thread;
};

struct UserInfo {
	std::string user;
	std::string pswd;
	int uid;
	std::string email;
};

/*
	对外调用接口
	通过sql_pool对象执行各种操作
*/
class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();

	//注册用户
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	
	//重置功能，检查是否可以重置
	//如果其他email没有绑定这个用户名且该email已经注册
	int CheckResetIsVaild(const std::string& name, const std::string& email);
	//重置功能，重置用户名和密码
	bool UpdateUserAndPswd(const std::string& name, const std::string& newpwd, const std::string& email);
	
	//登录功能，检查邮箱或用户名对应的密码是否正确
	bool LoginCheckPswd(const std::string& name_or_email, const std::string& pswd, UserInfo& userInfo);

private:
	std::unique_ptr<MysqlPool> sql_pool;
};