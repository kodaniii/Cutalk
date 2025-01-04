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

bool MysqlDao::AddFriendApply(int& from_id, int& to_id, std::string& reason) {
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
			"INSERT INTO friend_apply (from_uid, to_uid, apply_desc) values (?, ?, ?) "
			"ON DUPLICATE KEY UPDATE apply_desc = VALUES(apply_desc)"));
		pstmt->setInt(1, from_id); // from id
		pstmt->setInt(2, to_id);
		pstmt->setString(3, reason);

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
}

bool MysqlDao::GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList,
	int begin, int limit) {

	std::cout << "MysqlDao::GetApplyList()" << std::endl;

	auto conn = sql_pool->GetConnection();
	if (conn == nullptr) {
		return false;
	}

	Defer defer([this, &conn]() {
		sql_pool->PushConnection(std::move(conn));
		});


	try {
		/* 用户登陆时获取好友申请者的相关信息
		 * 获取的信息包括好友申请者的uid、用户名、昵称、性别、申请原因、我方是否已经同意该好友申请
		 */
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->sql_conn->prepareStatement(
			"select apply.from_uid, apply.status, apply.apply_desc, user.name, user.nick, user.sex "
			"from friend_apply as apply join user on apply.from_uid = user.uid "
			"where apply.to_uid = ? and apply.id > ? "
			"order by apply.id DESC "	//这里用的是降序，获取最新的limit(=10)个申请，ASC升序
			"LIMIT ? "));

		pstmt->setInt(1, touid);
		pstmt->setInt(2, begin);
		pstmt->setInt(3, limit);

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		while (res->next()) {
			auto name = res->getString("name");
			auto uid = res->getInt("from_uid");			
			std::cout << "Found a friend request originating from uid " << uid
				<< " in the MySQL database" << std::endl;
			auto apply_reason = res->getString("apply_desc");
			auto status = res->getInt("status");
			auto nick = res->getString("nick");
			auto sex = res->getInt("sex");
			auto apply_ptr = std::make_shared<ApplyInfo>(uid, name, apply_reason, "", nick, sex, status);
			// 最新的请求按照新旧排序存放到vector，新的在前，旧的在后，
			// Client端因为是前插item，所以要在Client再做一次逆置
			applyList.push_back(apply_ptr);
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << ", MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " " << std::endl;
		return false;
	}
}

bool MysqlDao::AuthFriendApply(const int& send_uid, const int& recv_uid, std::string recv_backname) {
	
	std::cout << "MysqlDao::AuthFriendApply()" << std::endl;
	
	auto conn = sql_pool->GetConnection();
	if (conn == nullptr) {
		return false;
	}

	Defer defer([this, &conn]() {
		sql_pool->PushConnection(std::move(conn));
		});

	try {
		//开始事务
		conn->sql_conn->setAutoCommit(false);

		//先更新friend_apply表，将status置1
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->sql_conn->prepareStatement(
			"UPDATE friend_apply SET status = 1 "
			"WHERE from_uid = ? AND to_uid = ?"));
		pstmt->setInt(1, send_uid);
		pstmt->setInt(2, recv_uid);

		int rowAffected = pstmt->executeUpdate();
		std::cout << "rowAffected1 = " << rowAffected << std::endl;
		if (rowAffected < 0) {
			conn->sql_conn->rollback();
			return false;
		}

		//插入被添加方好友数据
		std::unique_ptr<sql::PreparedStatement> pstmt2(conn->sql_conn->prepareStatement(
			"INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "
		));

		pstmt2->setInt(1, recv_uid);
		pstmt2->setInt(2, send_uid);
		pstmt2->setString(3, recv_backname);

		rowAffected = pstmt2->executeUpdate();
		std::cout << "rowAffected2 = " << rowAffected << std::endl;
		if (rowAffected < 0) {
			conn->sql_conn->rollback();
			return false;
		}

		//插入申请方好友数据
		std::unique_ptr<sql::PreparedStatement> pstmt3(conn->sql_conn->prepareStatement(
			"INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "
		));

		pstmt3->setInt(1, send_uid);
		pstmt3->setInt(2, recv_uid);
		//TODO 从apply_friend表查询反向好友申请的备注名back，再保存到这里
		//由于好友申请的mysql back添加没做，这里先略过
		pstmt3->setString(3, "");

		rowAffected = pstmt3->executeUpdate();
		std::cout << "rowAffected3 = " << rowAffected << std::endl;
		if (rowAffected < 0) {
			conn->sql_conn->rollback();
			return false;
		}


		conn->sql_conn->commit();
		std::cout << "Operation completed successfully" << std::endl;
		
		return true;

	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << ", MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " " << std::endl;
		return false;
	}
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