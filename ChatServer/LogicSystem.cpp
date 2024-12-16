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
				cout << "_msg_que.front() msg_id = " << msg_node->_recvnode->_msg_type_id << endl;
				auto call_back_iter = _func_callbacks.find(msg_node->_recvnode->_msg_type_id);

				//û�ҵ���Ӧ�Ļص�������ֱ��queue.pop()
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

		//������������
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