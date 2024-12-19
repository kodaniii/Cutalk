#ifndef TCPMGR_H
#define TCPMGR_H

#include <QTcpSocket>
#include "singleton.h"
#include "defs.h"
#include <functional>
#include <QObject>
#include "userdata.h"

class TcpMgr: public QObject, public Singleton<TcpMgr>, public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT

public:
    ~TcpMgr();

private:
    friend class Singleton<TcpMgr>;
    TcpMgr();
    void initHandlers();
    void handleMsg(ReqId id, int len, QByteArray data);
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    //处理TCP粘包问题，表示是否还有未处理的接收
    bool _b_recv_pending;
    //存储当前消息的type id和长度
    quint16 _message_type_id;
    quint16 _message_len;
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;

public slots:
    //http长连接请求
    void slot_tcp_connect(ServerInfo);
    //连接后发数据
    void slot_tcp_send_data(ReqId reqId, QByteArray data);

signals:
    //通知连接完成，b_succ表示是否成功
    void sig_tcp_conn_fin(bool b_success);
    //tcp连接到聊天服务器后，向聊天服务器发送数据
    void sig_tcp_send_data(ReqId reqId, QByteArray data);
    //通知切换聊天界面
    void sig_switch_chatdlg();
    //登录失败
    void sig_login_failed(int);


    //用户搜索
    void sig_user_search(std::shared_ptr<SearchInfo>);
};

#endif // TCPMGR_H
