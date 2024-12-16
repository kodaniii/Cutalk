#include "CSession.h"
#include "CServer.h"
#include <iostream>
#include <sstream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "LogicSystem.h"

using namespace std;

CSession::CSession(boost::asio::io_context& io_context, CServer* server)
	: _socket(io_context), 
	_server(server), 
	_isStop(false), 
	_b_head_parse(false), 
	_user_uid(0){
	cout << "CSession::CSession()" << endl;

	//boostѩ���㷨
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_session_id = boost::uuids::to_string(a_uuid);
	cout << "random_generator() _session_id " << _session_id << endl;

	//����һ��ͷ��Msg
	_recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

CSession::~CSession() {
	std::cout << "~CSession destruct" << endl;
}

tcp::socket& CSession::GetSocket() {
	return _socket;
}

std::string& CSession::GetSessionId() {
	return _session_id;
}

void CSession::SetUserId(int uid)
{
	_user_uid = uid;
}

int CSession::GetUserId()
{
	return _user_uid;
}

void CSession::Start(){
	AsyncReadHead(HEAD_TOTAL_LEN);
}

void CSession::Send(std::string msg, short msgid) {
	std::cout << "CSession::Send()" << std::endl;
	std::lock_guard<std::mutex> lk(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _session_id << ", send que fulled, size = " << MAX_SENDQUE << endl;
		return;
	}

	_send_que.push(make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Send(char* msg, short max_length, short msgid) {
	std::cout << "CSession::Send()" << std::endl;
	std::lock_guard<std::mutex> lk(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _session_id << ", send que fulled, size = " << MAX_SENDQUE << endl;
		return;
	}

	_send_que.push(make_shared<SendNode>(msg, max_length, msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len), 
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Close() {
	_socket.close();
	_isStop = true;
}

std::shared_ptr<CSession> CSession::SharedSelf() {
	return shared_from_this();
}

//��ȡTCP������ͷ����this->_recv_head_node
//��ͷ�����ݱ�����msg_id��msg_len��������������ֽ���Ϊ�����ֽ���
//��msg_id��msg_len������this->_recv_msg_node��Ȼ�������Ϣ��body
void CSession::AsyncReadHead(int total_len)
{
	cout << "CSession::AsyncReadHead()" << endl;
	auto self = shared_from_this();
	//asyncReadFull(size_t maxLen, std::function<...> handler);
	asyncReadFull(HEAD_TOTAL_LEN, 
		[self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error " << ec.what() << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			if (bytes_transfered < HEAD_TOTAL_LEN) {
				std::cout << "read length not match, read \"" << bytes_transfered << "\", total_len = "
					<< HEAD_TOTAL_LEN << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			//���յ���ͷ������this->_data������_recv_head_node->_data
			_recv_head_node->Clear();
			memcpy(_recv_head_node->_data, _data, bytes_transfered);

			//��_recv_head_node->_data��ȡͷ��msg_id
			short msg_id = 0;
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);

			//�����ֽ��򣨴�ˣ�ת��Ϊ�����ֽ���
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id = " << msg_id << endl;
			//id�Ƿ�
			if (msg_id > MAX_LENGTH) {
				std::cout << "invalid msg_id = " << msg_id << endl;
				_server->ClearSession(_session_id);
				return;
			}

			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			//�����ֽ���ת��Ϊ�����ֽ���
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len = " << msg_len << endl;

			//id�Ƿ�
			if (msg_len > MAX_LENGTH) {
				std::cout << "invalid msg_len = " << msg_len << endl;
				_server->ClearSession(_session_id);
				return;
			}

			//��ǰ4�ֽڱ�����_recv_msg_node
			_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
			AsyncReadBody(msg_len);
		}
		catch (std::exception& e) {
			std::cout << "Exception catch " << e.what() << endl;
		}
	});
}

void CSession::AsyncReadBody(int total_len)
{
	cout << "CSession::AsyncReadBody()" << endl;
	auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error " << ec.what() << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "read length not match, read \"" << bytes_transfered << "\", total_len = "
					<< total_len << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			//����TCP���������յ�������_data�洢��_recv_msg_node��_recv_msg_node���滻Ϊ��Ϣ��
			memcpy(_recv_msg_node->_data, _data, bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			cout << "receive data body " << _recv_msg_node->_data << endl;
			//�˴�����ϢͶ�ݵ��߼�������
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//��������ͷ�������¼�
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::cout << "Exception catch " << e.what() << endl;
		}
	});
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self) {
	std::cout << "CSession::HandleWrite()" << std::endl;
	//�����쳣����
	try {
		if (!error) {
			std::lock_guard<std::mutex> lk(_send_lock);
			//cout << "send data " << _send_que.front()->_data + HEAD_LENGTH << endl;
			_send_que.pop();
			//�ǿռ�������
			if (!_send_que.empty()) {
				auto& msgnode = _send_que.front();
				boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_self));
			}
		}
		else {
			std::cout << "handle write failed, error is " << error.what() << endl;
			Close();
			_server->ClearSession(_session_id);
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception catch " << e.what() << endl;
	}
}

//��ȡ��������
void CSession::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
	::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);
}

//��ȡָ���ֽ�����ʹ��async_read_some��װ
//async_read_some֧�ֶ�ȡָ�����ȣ�������ȫ����
//read_len��ʾ��ȡ�׵�ַ���൱���Ѿ���ȡ�ĳ��ȣ���total_len��ʾ�ܶ�ȡ���ȣ������Ѿ���ȡ�ĳ��ȣ�
//�����˳���������ص���������ec���Ѷ�����
void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len, 
	std::function<void(const boost::system::error_code&, std::size_t)> handler) {
	auto self = shared_from_this();
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len),
		[read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t bytesTransfered) {
			if (ec) {
				// ���ִ��󣬵��ûص�����
				handler(ec, read_len + bytesTransfered);
				return;
			}

			if (read_len + bytesTransfered >= total_len) {
				//���ȹ��˾͵��ûص�����
				handler(ec, read_len + bytesTransfered);
				return;
			}

			//û�д����ҳ��Ȳ����������ȡ
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
	});
}

LogicNode::LogicNode(shared_ptr<CSession> session, shared_ptr<RecvNode> recvnode)
	: _session(session), _recvnode(recvnode) {
	
}
