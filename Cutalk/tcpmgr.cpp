#include "tcpmgr.h"
#include <QAbstractSocket>
#include <QJsonDocument>
#include "usermgr.h"

TcpMgr::TcpMgr():
    _host(""),
    _port(0),
    _b_recv_pending(false),
    _message_type_id(0),
    _message_len(0) {

    //监听到来自_socket.connectToHost(_host, _port)的异步信号
    QObject::connect(&_socket, &QTcpSocket::connected, [&]() {
        qDebug() << "Connected to server!";
        emit sig_tcp_conn_fin(true);
    });

    //当有数据可读时，才读取所有数据，有效应对TCP粘包
    QObject::connect(&_socket, &QTcpSocket::readyRead, [&]() {

        _buffer.append(_socket.readAll());

        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_0);

        forever {
            //消息头处理
            if(!_b_recv_pending){
                // 检查缓冲区中的数据是否足够解析出一个消息头（消息ID + 消息长度）
                if (_buffer.size() < static_cast<int>(sizeof(quint16) * 2)) {
                    return; // 数据不够，等待更多数据
                }

                // 预读取消息ID和消息长度，但不从缓冲区中移除
                stream >> _message_type_id >> _message_len;

                //将buffer 中的前四个字节移除
                _buffer = _buffer.mid(sizeof(quint16) * 2);

                qDebug() << "QTcpSocket READ Message Type ID:" << _message_type_id << ", Length:" << _message_len;

            }

            //检查消息体长度是否满足条件
            //不满足则等待消息长度足够，下次循环直接跳转到这判断
            if(_buffer.size() < _message_len){
                _b_recv_pending = true;
                return;
            }

            /*处理消息体*/
            //读取完毕
            _b_recv_pending = false;

            // 读取消息体
            QByteArray messageBody = _buffer.mid(0, _message_len);
            qDebug() << "QTcpSocket Receive body msg" << messageBody;

            _buffer = _buffer.mid(_message_len);

            //_message_type_id
            handleMsg(ReqId(_message_type_id), _message_len, messageBody);
        }

    });

    //处理socket底层发生的错误
    //错误交给顶层处理
    QObject::connect(&_socket, static_cast<void (QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error),
                    [&](QTcpSocket::SocketError socketError) {
                        qDebug() << "Error:" << _socket.errorString();
                        switch (socketError) {
                        case QTcpSocket::ConnectionRefusedError:
                            qDebug() << "Connection Refused!";
                            emit sig_tcp_conn_fin(false);
                            break;
                        case QTcpSocket::RemoteHostClosedError:
                            qDebug() << "Remote Host Closed Connection!";
                            emit sig_tcp_conn_fin(false);
                            break;
                        case QTcpSocket::HostNotFoundError:
                            qDebug() << "Host Not Found!";
                            emit sig_tcp_conn_fin(false);
                            break;
                        case QTcpSocket::SocketTimeoutError:
                            qDebug() << "Connection Timeout!";
                            emit sig_tcp_conn_fin(false);
                            break;
                        case QTcpSocket::NetworkError:
                            qDebug() << "Network Error!";
                            break;
                        default:
                            qDebug() << "Other Error!";
                            break;
                        }
    });

    //连接断开提示
    QObject::connect(&_socket, &QTcpSocket::disconnected, [&]() {
        qDebug() << "Disconnected from server";
    });

    //tcp连接到聊天服务器后发送tcp登录消息
    QObject::connect(this, &TcpMgr::sig_tcp_send_data, this, &TcpMgr::slot_tcp_send_data);

    initHandlers();
}

TcpMgr::~TcpMgr(){

}

//QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;
void TcpMgr::handleMsg(ReqId req, int len, QByteArray data)
{
    qDebug() << "TcpMgr::handleMsg() Req" << req << ", len" << len << ", data" << data;
    auto find_iter = _handlers.find(req);
    if(find_iter == _handlers.end()){
        qDebug()<< "Not found Req" << req;
        emit sig_login_failed(StatusCodes::LoginHandlerFailed);
        return;
    }

    find_iter.value()(req, len, data);
}

void TcpMgr::initHandlers(){
    //这里不能使用CRTP延长this指针寿命
    //auto self = shared_from_this();
    //登录聊天服务器回包
    _handlers.insert(REQ_CHAT_LOGIN_RSP, [this](ReqId req, int len, QByteArray data){
        qDebug() << "TcpMgr::initHandlers() REQ_CHAT_LOGIN_RSP";
        Q_UNUSED(len);
        qDebug()<< "TcpMgr handle" << req;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument";
            emit sig_login_failed(StatusCodes::Error_Json);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug()<< "data jsonobj" << jsonObj;

        if(!jsonObj.contains("error")){
            int err = StatusCodes::Error_Json;
            qDebug() << "Json Parse Err" << err;
            emit sig_login_failed(err);
            return;
        }

        int stat = jsonObj["error"].toInt();
        if(stat != StatusCodes::Success){
            qDebug() << "Unknown Err" << stat;
            emit sig_login_failed(stat);
            return;
        }

        /*TODO USERMGR*/
        //data jsonobj QJsonObject({"error":0,"name":"1","token":"e37e82b8-d914-4cbf-820e-7a59847478f2","uid":0})
        //目前只实现了发name、token、uid和error
        auto uid = jsonObj["uid"].toInt();
        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();

        //登录用户的信息保存到UserMgr，绑定token
        auto user_info = std::make_shared<UserInfo>(uid, name, nick, icon, sex);
        UserMgr::GetInstance()->SetUserInfo(user_info);
        UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());

        //好友申请列表
        if(jsonObj.contains("apply_list")){
            UserMgr::GetInstance()->AppendApplyList(jsonObj["apply_list"].toArray());
        }

        //添加好友列表，FriendInfo
        if (jsonObj.contains("friend_list")) {
            UserMgr::GetInstance()->AppendFriendList(jsonObj["friend_list"].toArray());
        }


        //切换聊天界面
        emit sig_switch_chatdlg();
        qDebug() << "emit sig_switch_chatdlg()";
    });


    //搜索用户回包处理
    _handlers.insert(REQ_SEARCH_USER_RSP, [this](ReqId reqId, int len, QByteArray data) {
        qDebug() << "TcpMgr::initHandlers() REQ_SEARCH_USER_RSP";
        Q_UNUSED(len);
        qDebug()<< "TcpMgr handle" << reqId;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument";
            emit sig_user_search(false, nullptr);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug()<< "data jsonobj" << jsonObj;

        if(!jsonObj.contains("error")){
            int err = StatusCodes::Error_Json;
            qDebug() << "Json Parse Err" << err;
            emit sig_user_search(false, nullptr);
            return;
        }

        int stat = jsonObj["error"].toInt();

        if(stat == StatusCodes::UidInvalid){
            qDebug() << "Search handle success, but Cannot search uid";
            emit sig_user_search(true, nullptr);
            return;
        }
        if(stat != StatusCodes::Success){
            qDebug() << "Unknown Err" << stat;
            emit sig_user_search(false, nullptr);
            return;
        }

        QJsonDocument doc(jsonObj);
        QString jsonString = doc.toJson(QJsonDocument::Indented); // 格式化输出
        qDebug() << "search_info" << jsonString;

        /*这里的desc指的是搜索出来的个人信息的个人简介，但是这个功能没做显示*/
        auto search_info = std::make_shared<SearchInfo>(jsonObj["uid"].toInt(), jsonObj["name"].toString(),
                                                        jsonObj["nick"].toString(), jsonObj["desc"].toString(),
                                                        jsonObj["sex"].toInt(), jsonObj["icon"].toString());

        emit sig_user_search(true, search_info);
    });

    //添加联系人回包处理
    _handlers.insert(REQ_ADD_FRIEND_RSP, [this](ReqId reqId, int len, QByteArray data) {
        qDebug() << "TcpMgr::initHandlers() REQ_ADD_FRIEND_RSP";
        Q_UNUSED(len);
        qDebug()<< "TcpMgr handle" << reqId;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument";
            emit sig_user_search(false, nullptr);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug()<< "data jsonobj" << jsonObj;

        if(!jsonObj.contains("error")){
            int err = StatusCodes::Error_Json;
            qDebug() << "Json Parse Err" << err;
            emit sig_user_search(false, nullptr);
            return;
        }

        int stat = jsonObj["error"].toInt();

        if(stat != StatusCodes::Success){
            qDebug() << "Unknown Err" << stat;
            emit sig_user_search(false, nullptr);
            return;
        }

        qDebug() << "add_friend handle success";

    });

    //通知用户添加好友申请
    _handlers.insert(REQ_NOTIFY_ADD_FRIEND_REQ, [this](ReqId reqId, int len, QByteArray data) {
        qDebug() << "TcpMgr::initHandlers() REQ_NOTIFY_ADD_FRIEND_REQ";
        Q_UNUSED(len);
        qDebug()<< "TcpMgr handle" << reqId;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument";
            //直接借用用户搜索失败的界面
            //emit sig_user_search(false, nullptr);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug()<< "data jsonobj" << jsonObj;

        if(!jsonObj.contains("error")){
            int err = StatusCodes::Error_Json;
            qDebug() << "Json Parse Err" << err;
            //emit sig_user_search(false, nullptr);
            return;
        }

        int stat = jsonObj["error"].toInt();

        if(stat != StatusCodes::Success){
            qDebug() << "Unknown Err" << stat;
            //emit sig_user_search(false, nullptr);
            return;
        }


        int send_uid = jsonObj["send_uid"].toInt();
        QString name = jsonObj["send_name"].toString();
        QString desc = jsonObj["desc"].toString();
        QString back = jsonObj["back"].toString();
        QString icon = jsonObj["icon"].toString();
        QString nick = jsonObj["nick"].toString();
        int sex = jsonObj["sex"].toInt();

        auto apply_info = std::make_shared<AddFriendApply>(
            send_uid, name, desc,
            icon, nick, sex);

        emit sig_notify_friend_apply(apply_info);

    });

    //认证好友请求回包
    _handlers.insert(REQ_AUTH_FRIEND_RSP, [this](ReqId reqId, int len, QByteArray data) {
        qDebug() << "TcpMgr::initHandlers() REQ_AUTH_FRIEND_RSP";
        Q_UNUSED(len);
        qDebug()<< "TcpMgr handle" << reqId;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument";
            //直接借用用户搜索失败的界面
            emit sig_user_search(false, nullptr);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug()<< "data jsonobj" << jsonObj;

        if(!jsonObj.contains("error")){
            int err = StatusCodes::Error_Json;
            qDebug() << "Json Parse Err" << err;
            emit sig_user_search(false, nullptr);
            return;
        }

        int stat = jsonObj["error"].toInt();

        if(stat != StatusCodes::Success){
            qDebug() << "Unknown Err" << stat;
            emit sig_user_search(false, nullptr);
            return;
        }

        auto uid = jsonObj["send_uid"].toInt();
        auto name = jsonObj["send_name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();

        qDebug() << "[AuthRsp] uid" << uid
                 << "name" << name
                 << "nick" << nick
                 << "sex" << sex
                 << "icon" << icon;

        /*逻辑还没做icon的mysql查询，临时生成icon*/
        if(icon.isEmpty()){
            int randomValue = uid;
            int head_i = randomValue % heads.size();
            icon = heads[head_i];

            qDebug() << "icon" << icon;;
        }

        auto rsp = std::make_shared<AuthRsp>(uid, name, nick, icon, sex);

        bool isFriend = UserMgr::GetInstance()->CheckFriendById(rsp->_uid);
        //如果已经是好友了，添加好友无效
        if(isFriend){
            qDebug() << "isFriend ret true, return...";
            return;
        }

        //添加联系人项和聊天项
        //连接到contactuserlist新增联系人列表
        //连接到chatdialog新增聊天项
        emit sig_auth_rsp(rsp);
    });

    //通知用户对方认证了我方提出的好友请求
    //获取的是被申请方的信息
    _handlers.insert(REQ_NOTIFY_AUTH_FRIEND_REQ, [this](ReqId reqId, int len, QByteArray data) {
        qDebug() << "TcpMgr::initHandlers() REQ_NOTIFY_AUTH_FRIEND_REQ";
        Q_UNUSED(len);
        qDebug()<< "TcpMgr handle" << reqId;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument";
            //直接借用用户搜索失败的界面
            //emit sig_user_search(false, nullptr);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug()<< "data jsonobj" << jsonObj;

        if(!jsonObj.contains("error")){
            int err = StatusCodes::Error_Json;
            qDebug() << "Json Parse Err" << err;
            //emit sig_user_search(false, nullptr);
            return;
        }

        int stat = jsonObj["error"].toInt();

        if(stat != StatusCodes::Success){
            qDebug() << "Unknown Err" << stat;
            //emit sig_user_search(false, nullptr);
            return;
        }

        int recv_uid = jsonObj["recv_uid"].toInt();
        QString name = jsonObj["recv_name"].toString();
        QString nick = jsonObj["recv_nick"].toString();
        QString icon = jsonObj["recv_icon"].toString();
        QString back = jsonObj["recv_backname"].toString();
        int sex = jsonObj["recv_sex"].toInt();

        qDebug() << "[AuthNotify] uid" << recv_uid
                 << "name" << name
                 << "nick" << nick
                 << "sex" << sex
                 << "icon" << icon;

        /*逻辑还没做icon的mysql查询，临时生成icon*/
        if(icon.isEmpty()){
            int randomValue = recv_uid;
            int head_i = randomValue % heads.size();
            icon = heads[head_i];

            qDebug() << "icon" << icon;;
        }

        auto auth_info = std::make_shared<FriendInfo>(recv_uid, name, nick, icon,
                                                      sex, "", back, "");

        bool isFriend = UserMgr::GetInstance()->CheckFriendById(auth_info->_uid);
        qDebug() << "isFriend ret" << (isFriend? "true": "false");
        //如果已经是好友了，添加好友无效
        if(isFriend){
            qDebug() << "isFriend ret true, return...";
            emit sig_auth_rsp_set_btn_false(auth_info);
            return;
        }

        //添加联系人项和聊天项
        emit sig_add_auth_friend(auth_info);
        emit sig_auth_rsp_set_btn_false(auth_info);

    });


}

void TcpMgr::slot_tcp_send_data(ReqId reqId, QByteArray dataBytes)
{
    uint16_t id = reqId;

    quint16 len = static_cast<quint16>(dataBytes.length());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    //设置数据流使用网络字节序为大端模式
    out.setByteOrder(QDataStream::BigEndian);

    //使用QDataStream将id和len的值序列化为字节流，并写入到block
    out << id << len;

    //将传入的dataBytes追加到block的末尾
    block.append(dataBytes);

    //发送数据
    _socket.write(block);
    qDebug() << "TcpMgr::slot_tcp_send_data() send" << id << len << block;
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug() << "TcpMgr::slot_tcp_connect()";
    qDebug() << "Connecting to server...";
    _host = si.host;
    _port = static_cast<uint16_t>(si.port.toUInt());

    //异步连接
    //成功&QTcpSocket::connected
    _socket.connectToHost(_host, _port);
}
