#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"

LogicSystem::LogicSystem() {
	//Get测试
	RegisterGet("/get_test", [](std::shared_ptr<HttpConnection> conn) {
		std::cout << "/get_test get handler" << std::endl;
		beast::ostream(conn->_response.body()) << "receive get_test req" << std::endl;
		
		int i = 0;
		for (auto& elem : conn->_get_params) {
			i++;
			beast::ostream(conn->_response.body()) << "param " << i 
				<< ", key is " << elem.first
				<< ", value is " << elem.second << std::endl;
		}
	});

	//收取注册邮箱，grpc传给verifyserver，发送验证码
	RegisterPost("/get_verifycode", [](std::shared_ptr<HttpConnection> conn) {
		//从client得到post请求，获得email
		std::cout << "/get_verifycode post handler" << std::endl;
		auto body_str = boost::beast::buffers_to_string(conn->_request.body().data());
		std::cout << "_request.body().data() = " << body_str << std::endl;
		
		conn->_response.set(http::field::content_type, "text/json");

		Json::Value root;		//dst
		Json::Reader reader;
		Json::Value src_root;
		
		bool parse_succ = reader.parse(body_str, src_root);	//src_root <- (Json)body_str;
		//解析失败
		if (!parse_succ) {
			std::cout << "Failed to parse JSON data" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string json_str = root.toStyledString();
			beast::ostream(conn->_response.body()) << json_str;
			return;
		}

		//没有key
		if (!src_root.isMember("email")) {
			std::cout << "Failed to parse JSON data" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string json_str = root.toStyledString();
			beast::ostream(conn->_response.body()) << json_str;
			return;
		}

		//get the email
		auto email = src_root["email"].asString();
		std::cout << "src_root[\"email\"] = " << email << std::endl;

		//grpc client
		GetVerifyRsp rsp = VerifyGrpcClient::GetInstance()->GrpcVerifyCodeHandler(email);
		
		//TODO grpc client rsp
		root["error"] = rsp.error();
		root["email"] = src_root["email"];
		std::string json_str = root.toStyledString();
		beast::ostream(conn->_response.body()) << json_str;

		return;
	});

	//获取验证码
	//和Redis交互，检查验证码是否过期
	RegisterPost("/user_register", [](std::shared_ptr<HttpConnection> conn) {
		auto body_str = boost::beast::buffers_to_string(conn->_request.body().data());
		std::cout << "/user_register receive body " << body_str << std::endl;
		
		conn->_response.set(http::field::content_type, "text/json");

		Json::Value root;		//dst
		Json::Reader reader;
		Json::Value src_root;

		bool parse_succ = reader.parse(body_str, src_root);	//src_root <- (Json)body_str;
		if (!parse_succ) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		auto name = src_root["user"].asString();
		auto email = src_root["email"].asString();
		auto pswd = src_root["pswd"].asString();
		auto confirm_pswd = src_root["confirm_pswd"].asString();

		//后端再次检查确认密码是否正确
		if (pswd != confirm_pswd) {
			std::cout << "password err " << std::endl;
			root["error"] = ErrorCodes::PasswdErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		//检查redis中email对应的value
		std::string verify_code;
		//检查是否过期
		bool b_get_verify = RedisMgr::GetInstance()->Get(CODE_PREFIX + email, verify_code);
		std::cout << "RedisMgr Get " << CODE_PREFIX + email << " is " << verify_code << std::endl;
		if (!b_get_verify) {
			std::cout << "Get verify code expired" << std::endl;
			root["error"] = ErrorCodes::VerifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		//检查验证码是否正确
		if (verify_code != src_root["verifycode"].asString()) {
			std::cout << "Verify code error" << std::endl;
			root["error"] = ErrorCodes::VerifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		//TODO 查找MySQL数据库判断用户是否存在，is ok
		int uid = MysqlMgr::GetInstance()->RegUser(name, email, pswd);
		std::cout << "/user_register RegUser() ret uuid = " << uid << std::endl;
		if (uid == 0 || uid == -1) {
			std::cout << "user or email exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		/*
		/user_register receive body {
			"confirm_pswd": "1111",
			"email": "1111@qq.com",
			"pswd": "1111",
			"user": "1111",
			"verifycode": "6f90"
		}
		*/
		root["error"] = ErrorCodes::Success;	//额外添加
		root["uid"] = uid;						//额外添加
		root["email"] = src_root["email"].asString();
		root["user"] = src_root["user"].asString();
		root["pswd"] = src_root["pswd"].asString();
		root["confirm_pswd"] = src_root["confirm_pswd"].asString();
		root["verifycode"] = src_root["verifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(conn->_response.body()) << jsonstr;
		/*
		SEND body {
			"error": 0,
			"uid": 1,
			"confirm_pswd": "1111",
			"email": "1111@qq.com",
			"pswd": "1111",
			"user": "1111",
			"verifycode": "6f90"
		}
		*/
		return true;
		});

	//重置回调逻辑
	RegisterPost("/reset_user_and_passwd", [](std::shared_ptr<HttpConnection> conn) {
		auto body_str = boost::beast::buffers_to_string(conn->_request.body().data());
		std::cout << "/reset_user_and_passwd receive body " << body_str << std::endl;
		conn->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_succ = reader.parse(body_str, src_root);
		if (!parse_succ) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pswd = src_root["pswd"].asString();

		//先查找redis中email对应的验证码是否过期
		std::string verify_code;
		bool b_get_verify = RedisMgr::GetInstance()->Get(CODE_PREFIX + email, verify_code);
		std::cout << "RedisMgr Get " << CODE_PREFIX + email << " is " << verify_code << std::endl;
		if (!b_get_verify) {
			std::cout << "Get verify code expired" << std::endl;
			root["error"] = ErrorCodes::VerifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		//检查验证码是否正确
		if (verify_code != src_root["verifycode"].asString()) {
			std::cout << "Verify code error" << std::endl;
			root["error"] = ErrorCodes::VerifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		//查找MySQL数据库判断邮箱已经注册，且用户名是否可用
		int ret_reset_is_vaild = MysqlMgr::GetInstance()->CheckResetIsVaild(name, email);
		//-1：邮箱没有被注册过
		if (ret_reset_is_vaild == -1) {
			std::cout << "This email address has not been registered" << std::endl;
			root["error"] = ErrorCodes::EmailNotRegistered;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}
		//-2：用户名被其他用户占用
		else if (ret_reset_is_vaild == -2) {
			std::cout << "Username is already taken by another user" << std::endl;
			root["error"] = ErrorCodes::UsernameCannotUse;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		//更新密码为最新密码
		bool b_up = MysqlMgr::GetInstance()->UpdateUserAndPswd(name, pswd, email);
		if (!b_up) {
			std::cout << "Update username and passwd failed" << std::endl;
			root["error"] = ErrorCodes::ResetUpdateFailed;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		std::cout << "succeed to reset username = " << name << ", passwd = " << pswd << std::endl;

		root["error"] = ErrorCodes::Success;
		root["email"] = email;
		root["user"] = name;
		root["pswd"] = pswd;
		root["verifycode"] = src_root["verifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(conn->_response.body()) << jsonstr;
		return true;
		});
}

void LogicSystem::RegisterGet(std::string url, HttpHandler handler) {
	_get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::RegisterPost(std::string url, HttpHandler handler) {
	_post_handlers.insert(make_pair(url, handler));
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> conn) {
	//not found
	if (_get_handlers.find(path) == _get_handlers.end()) {
		return false;
	}

	_get_handlers[path](conn);
	
	return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> conn) {
	if (_post_handlers.find(path) == _post_handlers.end()) {
		return false;
	}

	_post_handlers[path](conn);

	return true;
}