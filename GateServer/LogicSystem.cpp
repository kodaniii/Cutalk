#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include "RedisMgr.h"

LogicSystem::LogicSystem() {
	//Get����
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

	//��ȡע�����䣬grpc����verifyserver��������֤��
	RegisterPost("/get_verifycode", [](std::shared_ptr<HttpConnection> conn) {
		//��client�õ�post���󣬻��email
		std::cout << "/get_verifycode post handler" << std::endl;
		auto body_str = boost::beast::buffers_to_string(conn->_request.body().data());
		std::cout << "_request.body().data() = " << body_str << std::endl;
		
		conn->_response.set(http::field::content_type, "text/json");

		Json::Value root;		//dst
		Json::Reader reader;
		Json::Value src_root;
		
		bool parse_succ = reader.parse(body_str, src_root);	//src_root <- (Json)body_str;
		//����ʧ��
		if (!parse_succ) {
			std::cout << "Failed to parse JSON data" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string json_str = root.toStyledString();
			beast::ostream(conn->_response.body()) << json_str;
			return;
		}

		//û��key
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

		auto email = src_root["email"].asString();
		auto pswd = src_root["pswd"].asString();
		auto confirm_pswd = src_root["confirm_pswd"].asString();

		//����ٴμ��ȷ�������Ƿ���ȷ
		if (pswd != confirm_pswd) {
			std::cout << "password err " << std::endl;
			root["error"] = ErrorCodes::PasswdErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		//���redis��email��Ӧ��value
		std::string verify_code;
		//����Ƿ����
		bool b_get_verify = RedisMgr::GetInstance()->Get(CODE_PREFIX + email, verify_code);
		std::cout << "RedisMgr Get " << CODE_PREFIX + email << " is " << verify_code << std::endl;
		if (!b_get_verify) {
			std::cout << "Get verify code expired" << std::endl;
			root["error"] = ErrorCodes::VerifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		//�����֤���Ƿ���ȷ
		if (verify_code != src_root["verifycode"].asString()) {
			std::cout << "Verify code error" << std::endl;
			root["error"] = ErrorCodes::VerifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
		}

		//TODO ����MySQL���ݿ��ж��û��Ƿ����


		root["error"] = ErrorCodes::Success;
		root["email"] = src_root["email"];
		root["user"] = src_root["user"].asString();
		root["pswd"] = src_root["pswd"].asString();
		root["confirm_pswd"] = src_root["confirm_pswd"].asString();
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