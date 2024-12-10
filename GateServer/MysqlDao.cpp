#include "MySQLDao.h"
#include "ConfigMgr.h"

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

/*
查询Mysql中的项，确保用户名和邮箱不存在的情况下，注册用户，返回uid
如果返回0，表示用户名或邮箱已存在
如果返回-1，表示错误
*/
int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pswd){
	auto conn = sql_pool->GetConnection();

	Defer defer([this, &conn]() {
		sql_pool->PushConnection(std::move(conn));
	});

	try {
		if (conn == nullptr) {
			//这行是BUG，空连接是isStop情况下凭空产生的，不能move回去
			//sql_pool->PushConnection(std::move(conn));
			return false;
		}
		//SQL预编译
		// 准备调用存储过程
		std::unique_ptr<sql::PreparedStatement> stmt(conn->sql_conn->prepareStatement("CALL reg_user(?,?,?,@result)"));
		// 设置输入参数
		//setString(int parameterIndex, String x), setInt(int parameterIndex, int value)
		stmt->setString(1, name);
		stmt->setString(2, email);
		stmt->setString(3, pswd);

		// 使用会话变量或其他方法来获取输出参数的值

		// 执行存储过程
		stmt->execute();
		// 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
		// 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
		std::unique_ptr<sql::Statement> stmtResult(conn->sql_conn->createStatement());
		//获取uid
		/*
		0：表示用户名或邮箱已存在。
		-1：表示在执行过程中遇到错误。
		其他值（如@new_id）：表示新插入的用户ID，即注册成功。
		*/
		std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
		if (res->next()) {
			int result = res->getInt("result");
			std::cout << "MysqlDao::RegUser() Get uuid: " << result << std::endl;
			return result;
		}
		return -1;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << ", MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " " << std::endl;
		return -1;
	}
}

int MysqlDao::CheckResetIsVaild(const std::string& name, const std::string& email) {
	auto conn = sql_pool->GetConnection();

	Defer defer([this, &conn]() {
		sql_pool->PushConnection(std::move(conn));
	});

	try {
		if (conn == nullptr) {
			return -3;	//数据库操作失败
		}

		//检查邮箱是否存在
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->sql_conn->prepareStatement("SELECT COUNT(*) FROM user WHERE email = ?"));
		pstmt->setString(1, email);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		if (!res->next() || res->getInt(1) == 0) {
			return -1;	//邮箱不存在
		}

		//检查用户名是否被其他邮箱占用
		//查询规则WHERE：除了当前email的所有条目中是否有name = name的条目
		pstmt.reset(conn->sql_conn->prepareStatement("SELECT name FROM user WHERE name = ? AND email <> ?"));
		pstmt->setString(1, name);
		pstmt->setString(2, email);
		res.reset(pstmt->executeQuery());
		if (res->next()) {
			return -2;	//用户名被其他邮箱占用
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << ", MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " " << std::endl;
		return -3;	//数据库操作失败
	}
}

bool MysqlDao::UpdateUserAndPswd(const std::string& name, const std::string& pswd, const std::string& email) {
	auto conn = sql_pool->GetConnection();

	Defer defer([this, &conn]() {
		sql_pool->PushConnection(std::move(conn));
	});

	try {
		if (conn == nullptr) {
			return false;
		}

		// 准备查询语句
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->sql_conn->prepareStatement("UPDATE user SET name = ?, pswd = ? WHERE email = ?"));

		// 绑定参数
		pstmt->setString(1, name);
		pstmt->setString(2, pswd);
		pstmt->setString(3, email);

		// 执行更新
		int updateCount = pstmt->executeUpdate();

		std::cout << "Reset rows: " << updateCount << std::endl;
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << ", MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " " << std::endl;
		return false;
	}
}

bool MysqlDao::LoginCheckPswd(const std::string& login_key, const std::string& pwd, UserInfo& userInfo) {
	auto conn = sql_pool->GetConnection();
	if (conn == nullptr) {
		return false;
	}

	Defer defer([this, &conn]() {
		sql_pool->PushConnection(std::move(conn));
	});

	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->sql_conn->prepareStatement("SELECT * FROM user WHERE name = ? OR email = ?"));
		pstmt->setString(1, login_key); // 第一个参数尝试匹配用户名
		pstmt->setString(2, login_key); // 第二个参数尝试匹配邮箱

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::string correct_pwd = "";

		if (res->next()) {
			correct_pwd = res->getString("pswd"); // 确保字段名与数据库中的字段名匹配
			std::cout << "Login user passwd: " << correct_pwd << std::endl;
			std::cout << "Login input passwd: " << pwd << std::endl;
		}
		//结果为空
		else {
			return false;
		}

		if (pwd != correct_pwd) {
			return false;
		}
		userInfo.user = res->getString("name");
		userInfo.pswd = correct_pwd;
		userInfo.email = res->getString("email");
		userInfo.uid = res->getInt("uid");
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