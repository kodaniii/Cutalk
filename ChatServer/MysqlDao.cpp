#include "MySQLDao.h"
#include "ConfigMgr.h"
#include "data.h"

MysqlDao::MysqlDao() {
	auto& cfg = ConfigMgr::init();
	const auto& host = cfg["Mysql"]["host"];
	const auto& port = cfg["Mysql"]["port"];
	const auto& pswd = cfg["Mysql"]["pswd"];
	const auto& schema = cfg["Mysql"]["schema"];
	const auto& user = cfg["Mysql"]["user"];
	sql_pool.reset(new MysqlPool(host + ":" + port, user, pswd, schema, 5));
	std::cout << "MysqlDao()" << std::endl;
}

MysqlDao::~MysqlDao() {
	sql_pool->Close();
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(int uid)
{
	std::cout << "MysqlDao::GetUser(uid)" << std::endl;
	auto conn = sql_pool->GetConnection();
	if (conn == nullptr) {
		return nullptr;
	}

	Defer defer([this, &conn]() {
		sql_pool->PushConnection(std::move(conn));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->sql_conn->prepareStatement("SELECT * FROM user WHERE uid = ?"));
		pstmt->setInt(1, uid);

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::shared_ptr<UserInfo> user_ptr = nullptr;

		while (res->next()) {
			user_ptr.reset(new UserInfo);
			user_ptr->pswd = res->getString("pswd");
			user_ptr->email = res->getString("email");
			user_ptr->name = res->getString("name");
			user_ptr->nick = res->getString("nick");
			user_ptr->desc = res->getString("desc");
			user_ptr->sex = res->getInt("sex");
			user_ptr->icon = res->getString("icon");
			user_ptr->uid = uid;
			std::cout << "MysqlDao::GetUser uid " << uid << std::endl;
			break;
		}
		return user_ptr;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << ", MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " " << std::endl;
		return nullptr;
	}
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(std::string name)
{
	std::cout << "MysqlDao::GetUser(name)" << std::endl;
	auto conn = sql_pool->GetConnection();
	if (conn == nullptr) {
		return nullptr;
	}

	Defer defer([this, &conn]() {
		sql_pool->PushConnection(std::move(conn));
	});

	try {
		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->sql_conn->prepareStatement("SELECT * FROM user WHERE name = ?"));
		pstmt->setString(1, name); // 将uid替换为你要查询的uid

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::shared_ptr<UserInfo> user_ptr = nullptr;
		// 遍历结果集
		while (res->next()) {
			user_ptr.reset(new UserInfo);
			user_ptr->pswd = res->getString("pswd");
			user_ptr->email = res->getString("email");
			user_ptr->name = res->getString("name");
			user_ptr->nick = res->getString("nick");
			user_ptr->desc = res->getString("desc");
			user_ptr->sex = res->getInt("sex");
			user_ptr->icon = res->getString("icon");
			user_ptr->uid = res->getInt("uid");
			break;
		}
		return user_ptr;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << ", MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " " << std::endl;
		return nullptr;
	}
}

bool MysqlDao::AddFriendApply(int& from_id, int& to_id) {
	std::cout << "MysqlDao::AddFriendApply()" << std::endl;
	auto conn = sql_pool->GetConnection();
	if (conn == nullptr) {
		return false;
	}

	Defer defer([this, &conn]() {
		sql_pool->PushConnection(std::move(conn));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->sql_conn->prepareStatement(
			"INSERT INTO friend_apply (from_uid, to_uid) values (?, ?) "
			"ON DUPLICATE KEY UPDATE from_uid = from_uid, to_uid = to_uid"));
		pstmt->setInt(1, from_id); // from id
		pstmt->setInt(2, to_id);

		int rowAffected = pstmt->executeUpdate();
		//rowAffected = 0，表示这个条目被更新
		//			  = 1，表示这个条目新添加
		if (rowAffected < 0) {
			return false;
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << ", MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " " << std::endl;
		return false;
	}


	return true;
}

MysqlPool::MysqlPool(const std::string& _url, const std::string& _user, const std::string& _pswd, const std::string& _schema, int _poolSize)
	: url(_url), user(_user), pswd(_pswd), schema(_schema), poolSize(_poolSize), isStop(false) {
	std::cout << "MysqlPool(url, user, pswd, schema, poolSize) = {"
		<< url << ", " << user << ", " << pswd << ", " << schema << ", " << poolSize 
		<< "}" << std::endl;
	try {
		for (int i = 0; i < poolSize; ++i) {
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			auto *sql_conn = driver->connect(url, user, pswd);
			sql_conn->setSchema(schema);

			// 获取当前时间戳
			auto curTime = std::chrono::system_clock::now().time_since_epoch();
			// 将时间戳转换为秒
			long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(curTime).count();

			//MysqlConnection::MysqlConnection(sql_conn, last_time)
			conns.push(std::make_unique<MysqlConnection>(sql_conn, timestamp));
		}

		//每60秒激活一次checkConnection()
		detach_thread = std::thread([this]() {
			while (!isStop) {
				CheckConnection();
				std::this_thread::sleep_for(std::chrono::seconds(60));
			}
		});
		
		//后台执行
		detach_thread.detach();
	}
	catch (sql::SQLException& e) {
		// 处理异常
		std::cout << "mysql pool init failed, err is " << e.what() << std::endl;
	}
}

void MysqlPool::CheckConnection() {
	std::cout << "MysqlPool::CheckConnection()" << std::endl;
	std::lock_guard<std::mutex> lk(mtx);
	int poolsize = conns.size();
	// 获取当前时间戳
	auto cutTime = std::chrono::system_clock::now().time_since_epoch();
	// 将时间戳转换为秒
	long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(cutTime).count();
	for (int i = 0; i < poolsize; i++) {
		auto conn = std::move(conns.front());
		conns.pop();
		Defer defer([this, &conn]() {
			conns.push(std::move(conn));
		});

		if (timestamp - conn->last_op_time < 5) {
			continue;
		}

		try {
			std::unique_ptr<sql::Statement> stmt(conn->sql_conn->createStatement());
			stmt->executeQuery("SELECT 1");
			conn->last_op_time = timestamp;
			//std::cout << "execute timer alive query , cur is " << timestamp << std::endl;
		}
		catch (sql::SQLException& e) {
			std::cout << "Error keeping connection alive: " << e.what() << std::endl;
			// 重新创建连接并替换旧的连接
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			auto* new_conn = driver->connect(url, user, pswd);
			new_conn->setSchema(schema);
			conn->sql_conn.reset(new_conn);
			conn->last_op_time = timestamp;
		}
	}
}

std::unique_ptr<MysqlConnection> MysqlPool::GetConnection() {
	std::unique_lock<std::mutex> lk(mtx);
	cond.wait(lk, [this]() {
		if (isStop) {
			return true;
		}
		return !conns.empty(); 
	});
	if (isStop) {
		return nullptr;
	}
	std::unique_ptr<MysqlConnection> conn(std::move(conns.front()));
	conns.pop();
	return conn;
}

void MysqlPool::PushConnection(std::unique_ptr<MysqlConnection> conn) {
	std::lock_guard<std::mutex> lk(mtx);
	if (isStop) {
		return;
	}
	conns.push(std::move(conn));
	cond.notify_one();
}

void MysqlPool::Close() {
	isStop = true;
	cond.notify_all();
}

MysqlPool::~MysqlPool() {
	std::lock_guard<std::mutex> lk(mtx);
	while (!conns.empty()) {
		conns.pop();
	}
}