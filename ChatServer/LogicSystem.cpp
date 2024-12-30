#include "LogicSystem.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "defs.h"
#include "StatusGrpcClient.h"
#include "UserMgr.h"

#define forever for(;;)

using namespace std;

LogicSystem::LogicSystem() 
	: _isStop(false) {
	std::cout << "LogicSystem::LogicSystem()" << std::endl;
	RegisterCallBacks();
	_work_thread = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem() {
	_isStop = true;
	_cond.notify_one();
	_work_thread.join();
}

void LogicSystem::PostMsgToQue(shared_ptr<LogicNode> msg) {
	std::unique_lock<std::mutex> lk(_mtx);
	//��Ϣ�ŵ�������
	_msg_que.push(msg);
	
	//���_msg_queԭ���ǿյģ����ڷ�����һ����˵���̱߳��������������������̴߳���
	if (_msg_que.size() == 1) {
		lk.unlock();
		_cond.notify_one();
	}
}

//��������е���Ϣ
void LogicSystem::DealMsg() {
	std::cout << "LogicSystem::DealMsg()" << std::endl;
	forever {
		std::unique_lock<std::mutex> lk(_mtx);
		//�ж϶���Ϊ�գ��������������ȴ������ͷ���
		//ֱ������ʱ���ѻ���б�������Ϣʱ����
		while (_msg_que.empty() && !_isStop) {
			_cond.wait(lk);
		}

		//�ر�״̬����ʣ����Ϣȫ������
		if (_isStop) {
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				cout << "_msg_que.front() msg_type_id = " << msg_node->_recvnode->_msg_type_id << endl;
				auto call_back_iter = _func_callbacks.find(msg_node->_recvnode->_msg_type_id);

				//û�ҵ���Ӧ�Ļص�������ֱ��queue.pop()
				if (call_back_iter == _func_callbacks.end()) {
					std::cout << "cannot found _func_callbacks[msg_type_id]" << std::endl;
					_msg_que.pop();
					continue;
				}

				//���ö�Ӧ�Ļص�����
				call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_type_id,
					std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
				_msg_que.pop();
			}
			break;
		}

		//������������
		auto msg_node = _msg_que.front();
		cout << "_msg_que.front() msg_type_id = " << msg_node->_recvnode->_msg_type_id << endl;
		auto call_back_iter = _func_callbacks.find(msg_node->_recvnode->_msg_type_id);

		if (call_back_iter == _func_callbacks.end()) {
			std::cout << "cannot found _func_callbacks[msg_type_id]" << std::endl;
			_msg_que.pop();
			continue;
		}

		call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_type_id,
			std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
		_msg_que.pop();
	}
}

void LogicSystem::RegisterCallBacks() {
	_func_callbacks[REQ_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

}

/* ��ѯ�û���Ϣ
 * �Ȳ�ѯRedis�����û���ٲ�ѯMysql
 */
bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	//���Ȳ�redis�в�ѯ�û���Ϣ
	std::string info_str = "";
	bool b_base = RedisMgr::GetInstance()->Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		userinfo->uid = root["uid"].asInt();
		userinfo->name = root["name"].asString();
		userinfo->pswd = root["pswd"].asString();
		userinfo->email = root["email"].asString();
		userinfo->nick = root["nick"].asString();
		userinfo->desc = root["desc"].asString();
		userinfo->sex = root["sex"].asInt();
		userinfo->icon = root["icon"].asString();
		std::cout << "user login uid " << userinfo->uid << ", name "
			<< userinfo->name << ", pswd " << userinfo->pswd << ", email " << userinfo->email << endl;
	}
	else {
		//redis��û�����ѯmysql
		//��ѯ���ݿ�
		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}

		userinfo = user_info;

		//�����ݿ�����д��redis����
		Json::Value redis_root;
		redis_root["uid"] = uid;
		redis_root["pswd"] = userinfo->pswd;
		redis_root["name"] = userinfo->name;
		redis_root["email"] = userinfo->email;
		redis_root["nick"] = userinfo->nick;
		redis_root["desc"] = userinfo->desc;
		redis_root["sex"] = userinfo->sex;
		redis_root["icon"] = userinfo->icon;
		RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
	}

	return true;
}

//�û���¼������ȡ��ϵ�ˡ���������ȸ�����Ϣ
void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short& msg_type_id, const string& msg_data) {
	std::cout << "LogicSystem::LoginHandler()" << std::endl;

	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "user login uid " << uid << ", user token " << token << endl;

	Json::Value rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, REQ_CHAT_LOGIN_RSP);
	});

	//��redis��ȡ�û�token�Ƿ���ȷ
	std::string uid_str = std::to_string(uid);
	std::string token_key = USER_TOKEN_PREFIX + uid_str;
	std::string token_value = "";
	bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
	if (!success) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	if (token_value != token) {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
	}

	rtvalue["error"] = ErrorCodes::Success;


	/*��ȡ�û�������Ϣ*/
	std::string base_key = USER_BASE_INFO + uid_str;
	auto user_info = std::make_shared<UserInfo>();
	bool b_base = GetBaseInfo(base_key, uid, user_info);
	if (!b_base) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	rtvalue["uid"] = uid;
	rtvalue["pswd"] = user_info->pswd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;

	//TODO ��ȡ�����б�

	//TODO ��ȡ���������б�

	/* ���·�������¼��������StatusServer���ؾ���ʹ��
	 * �����������ڷ�����������ʱ��ͳ�ʼ����
	 */
	auto server_name = ConfigMgr::init().GetValue("SelfServer", "name");

	/*
	auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
	int login_cnt = 0;
	if (!rd_res.empty()) {
		login_cnt = std::stoi(rd_res);
	}
	login_cnt++;
	auto count_str = std::to_string(login_cnt);
	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);*/

	bool isSucc = RedisMgr::GetInstance()->HAdd(LOGIN_COUNT, server_name);
	std::cout << "RedisMgr::GetInstance()->HAdd() " << (isSucc ? "success" : "fail") << std::endl;


	/*�󶨵�ǰTCP session���û�uid*/
	session->SetUserId(uid);

	/* ���û�uid���ڵ�ChatServer name���浽Redis��
	 * ���ڶ�λ�û��������������ip��port��
	 * ���ڿ������ͨ�ţ�����ֱ���ҵ�Ŀ���û������������
	 */
	std::string uid_ip = USER_IP_PREFIX + uid_str;
	RedisMgr::GetInstance()->Set(uid_ip, server_name);

	/* ��uid��TCP session��¼��UserMgr _uid_to_session��
	 * ʵ��uid��sessionӳ�䣬����ͨ��CServer::ClearSession(session_id)ʵ������(�Ͽ�����)����
	 * CServer::ClearSession(session_id)�����UserMgr��uid��session��ӳ�䣬Ȼ�����session����
	 */
	UserMgr::GetInstance()->SetUserSession(uid, session);

	return;



	/*	//�����Ƕ�ChatServer��ͳһ���õ�Redis�ϣ������ڱ����ڴ���
	//��״̬��������ȡtokenƥ���Ƿ�׼ȷ
	//LoginReq(uid, token) -> LoginRsp(error, uid, token)
	auto rsp = StatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
	std::cout << "StatusGrpcClient Login rsp error " << rsp.error() 
		<< ", uid " << rsp.uid() << ", token " << rsp.token() << std::endl;


	rtvalue["error"] = rsp.error();
	if (rsp.error() != ErrorCodes::Success) {
		return;
	}*/


	/*	//�ⲿ�ִ����װ��LogicSystem::GetBaseInfo()
	//�ڴ��в�ѯ�û���Ϣ
	auto find_iter = _users.find(uid);
	std::shared_ptr<UserInfo> user_info = nullptr;
	//�û���Ϣ�����ڴ���
	if (find_iter == _users.end()) {
		//ͨ��uid��ѯ���ݿ�
		user_info = MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			std::cout << "Cannot found uid " << uid << " in Mysql user table" << std::endl;
			rtvalue["error"] = ErrorCodes::UidInvalid;
			return;
		}

		_users[uid] = user_info;
	}
	else {
		user_info = find_iter->second;
	}

	rtvalue["uid"] = uid;
	rtvalue["token"] = rsp.token();
	rtvalue["name"] = user_info->name;*/

}