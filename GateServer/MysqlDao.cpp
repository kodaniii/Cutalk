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
			sql_pool->PushConnection(std::move(conn));
			return result;
		}
		sql_pool->PushConnection(std::move(conn));
		return -1;
	}
	catch (sql::SQLException& e) {
		sql_pool->PushConnection(std::move(conn));
		std::cerr << "SQLException: " << e.what();
		std::cerr << ", MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " " << std::endl;
		return -1;
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