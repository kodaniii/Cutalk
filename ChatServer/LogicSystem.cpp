#include "LogicSystem.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"

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
				cout << "_msg_que.front() msg_id = " << msg_node->_recvnode->_msg_type_id << endl;
				auto call_back_iter = _func_callbacks.find(msg_node->_recvnode->_msg_type_id);

				//没找到对应的回调函数，直接queue.pop()
				if (call_back_iter == _func_callbacks.end()) {
					_msg_que.pop();
					continue;
				}

				call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_type_id,
					std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
				_msg_que.pop();
			}
			break;
		}

		//队列中有数据
		auto msg_node = _msg_que.front();
		cout << "_msg_que.front() msg_id = " << msg_node->_recvnode->_msg_type_id << endl;
		auto call_back_iter = _func_callbacks.find(msg_node->_recvnode->_msg_type_id);

		if (call_back_iter == _func_callbacks.end()) {
			_msg_que.pop();
			continue;
		}

		call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_type_id,
			std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
		_msg_que.pop();
	}
}

void LogicSystem::RegisterCallBacks() {

}