#include "LogicSystem.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "defs.h"
#include "StatusGrpcClient.h"

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

//�û���¼����
void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short& msg_type_id, const string& msg_data) {
	std::cout << "LogicSystem::LoginHandler()" << std::endl;

	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	std::cout << "user login uid " << uid << ", user token " << root["token"].asString() << endl;

	//��״̬��������ȡtokenƥ���Ƿ�׼ȷ
	//LoginReq(uid, token) -> LoginRsp(error, uid, token)
	auto rsp = StatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
	std::cout << "StatusGrpcClient Login rsp error " << rsp.error() 
		<< ", uid " << rsp.uid() << ", token " << rsp.token() << std::endl;
	Json::Value rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, REQ_CHAT_LOGIN_RSP);
	});

	rtvalue["error"] = rsp.error();
	if (rsp.error() != ErrorCodes::Success) {
		return;
	}

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
	rtvalue["name"] = user_info->name;
}