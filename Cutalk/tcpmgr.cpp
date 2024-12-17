#include "tcpmgr.h"
#include <QAbstractSocket>
#include <QJsonDocument>

TcpMgr::TcpMgr(): _host(""), _port(0), _b_recv_pending(false), _message_type_id(0), _message_len(0) {

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
            qDebug() << "QTcpSocket Receive body msg" << messageBody ;

            _buffer = _buffer.mid(_message_len);

            //_message_type_id
            handleMsg(ReqId(_message_type_id), _message_len, messageBody);
        }

    });

    //处理socket底层发生的错误
    //错误交给顶层处理
    QObject::connect(&_socket, static_cast<void (QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error),
                    [&](QTcpSocket::SocketError socketError) {
                        qDebug() << "Error:" << _socket.errorString() ;
                        switch (socketError) {
                        case QTcpSocket::ConnectionRefusedError:
                            qDebug() << "Connection Refused!";
                            emit sig_tcp_conn_fin(false);
                            break;
                        case QTcpSocket::RemoteHostClosedError:
                            qDebug() << "Remote Host Closed Connection!";
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
        return ;
    }

    find_iter.value()(req, len, data);
}

void TcpMgr::initHandlers(){
    //这里不能使用CRTP延长this指针寿命
    //auto self = shared_from_this();
    //登录聊天服务器回包
    _handlers.insert(REQ_CHAT_LOGIN_RSP, [this](ReqId req, int len, QByteArray data){
        Q_UNUSED(len);
        qDebug()<< "TcpMgr handle"<< req ;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument";
            emit sig_login_failed(StatusCodes::Error_Json);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug()<< "data jsonobj" << jsonObj ;

        if(!jsonObj.contains("error")){
            int err = StatusCodes::Error_Json;
            qDebug() << "Json Parse Err" << err ;
            emit sig_login_failed(err);
            return;
        }

        int stat = jsonObj["error"].toInt();
        if(stat != StatusCodes::Success){
            qDebug() << "Unknown Err" << stat ;
            emit sig_login_failed(stat);
            return;
        }

        //TODO USERMGR

        //切换聊天界面
        emit sig_switch_chatdlg();
        qDebug() << "emit sig_switch_chatdlg()";
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
