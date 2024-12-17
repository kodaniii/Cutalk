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
	//消息放到队列中
	_msg_que.push(msg);
	
	//如果_msg_que原本是空的，现在放入了一个，说明线程被条件变量阻塞，唤醒线程处理
	if (_msg_que.size() == 1) {
		lk.unlock();
		_cond.notify_one();
	}
}

//处理队列中的消息
void LogicSystem::DealMsg() {
	std::cout << "LogicSystem::DealMsg()" << std::endl;
	forever {
		std::unique_lock<std::mutex> lk(_mtx);
		//判断队列为空，条件变量阻塞等待，并释放锁
		//直到析构时唤醒或队列被放入消息时唤醒
		while (_msg_que.empty() && !_isStop) {
			_cond.wait(lk);
		}

		//关闭状态，把剩余消息全部发送
		if (_isStop) {
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				cout << "_msg_que.front() msg_type_id = " << msg_node->_recvnode->_msg_type_id << endl;
				auto call_back_iter = _func_callbacks.find(msg_node->_recvnode->_msg_type_id);

				//没找到对应的回调函数，直接queue.pop()
				if (call_back_iter == _func_callbacks.end()) {
					std::cout << "cannot found _func_callbacks[msg_type_id]" << std::endl;
					_msg_que.pop();
					continue;
				}

				//调用对应的回调函数
				call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_type_id,
					std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
				_msg_que.pop();
			}
			break;
		}

		//队列中有数据
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

//用户登录处理
void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short& msg_type_id, const string& msg_data) {
	std::cout << "LogicSystem::LoginHandler()" << std::endl;

	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	std::cout << "user login uid " << uid << ", user token " << root["token"].asString() << endl;

	//从状态服务器获取token匹配是否准确
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

	//内存中查询用户信息
	auto find_iter = _users.find(uid);
	std::shared_ptr<UserInfo> user_info = nullptr;
	//用户信息不在内存中
	if (find_iter == _users.end()) {
		//通过uid查询数据库
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