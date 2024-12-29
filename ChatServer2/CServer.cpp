#include "CServer.h"
#include "AsioIOServicePool.h"
#include "usermgr.h"

using namespace std;
using boost::asio::ip::tcp;

CServer::CServer(boost::asio::io_context& ioc, short port)
	: _ioc(ioc),
	_acceptor(ioc, tcp::endpoint(tcp::v4(), port)){

	cout << "CServer::CServer() port " << port << endl;

	//监听ioc start
	Start();
}

CServer::~CServer(){
	cout << "CServer::~CServer()" << endl;
}

void CServer::Start() {
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	shared_ptr<CSession> new_session = make_shared<CSession>(io_context, this);
	_acceptor.async_accept(new_session->GetSocket(), 
		std::bind(&CServer::HandleAccept, this, new_session, placeholders::_1));

}

void CServer::HandleAccept(shared_ptr<CSession> new_session, const boost::system::error_code& error) {
	cout << "CServer::HandleAccept()" << endl;
	if (!error) {
		new_session->Start();
		lock_guard<mutex> lk(_mtx);
		_sessions.insert(make_pair(new_session->GetSessionId(), new_session));
	}
	else {
		cout << "session accept failed, error is " << error.what() << endl;
	}

	Start();
}

void CServer::ClearSession(std::string session_id) {
	cout << "CServer::ClearSession() session_id = " << session_id << endl;

	if (_sessions.find(session_id) != _sessions.end()) {
		//移除用户和session的关联
		UserMgr::GetInstance()->RemoveUserSession(_sessions[session_id]->GetUserId());
	}

	{
		lock_guard<mutex> lk(_mtx);
		_sessions.erase(session_id);
	}
}
