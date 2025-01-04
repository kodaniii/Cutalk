#pragma once
#include "defs.h"
#include <thread>
#include "data.h"
#include <string>

/*
	ά��sql_conn��last_op_time
	last_op_time:	���������һ�β�����ʱ��
	���ڷ���queue<std::unique_ptr<MysqlConnection>> conns
*/
class MysqlConnection {
public:
	MysqlConnection(sql::Connection* _conn, int64_t last_time) 
		: sql_conn(_conn), last_op_time(last_time){	}

	std::unique_ptr<sql::Connection> sql_conn;
	int64_t last_op_time;
};

/*
	ά��sql_conn��user��pswd��url��schema����Ϣ
	MysqlConnection��
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
	//ÿһ��conn����һ��MysqlConnection
	//MysqlConnection::MysqlConnection(sql_conn, last_time)
	//conn->sql_conn
	std::queue<std::unique_ptr<MysqlConnection>> conns;
	std::mutex mtx;
	std::condition_variable cond;

	std::thread detach_thread;
};

/*
	������ýӿ�
	ͨ��sql_pool����ִ�и��ֲ���
*/
class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();

	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUser(std::string name);


	bool AddFriendApply(int& from_id, int& to_id, std::string& reason);
	bool GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit);
	bool AuthFriendApply(const int& send_uid, const int& recv_uid, std::string recv_backname);


private:
	std::unique_ptr<MysqlPool> sql_pool;
};