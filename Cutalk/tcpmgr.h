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
    //TCP连接发数据，接收数据在initHandlers()里面处理
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


    //用户搜索，已经搜索到用户，存储到SearchInfo对象中
    void sig_user_search(bool, std::shared_ptr<SearchInfo>);
    /*对方发出好友申请信号，处理相关好友申请item显示*/
    void sig_notify_friend_apply(std::shared_ptr<AddFriendApply>);
    /*TCPMGR：接收到好友验证请求*/
    void sig_add_auth_friend(std::shared_ptr<FriendInfo>);
    /*TCPMGR：发送的好友验证请求收到对方回复*/
    void sig_auth_rsp(std::shared_ptr<AuthRsp>);
    /*接受tcp传递的authrsp信号处理，对于双方已经是好友的情况，只需要将同意添加好友按钮置为已同意
    //发生的情况是：双方相互发好友申请，然后有一方接受了好友申请，对方没接受的情况
    //这里先这么做，后面直接修改Mysql，把双方的申请都置1，做双重判断*/
    //置添加好友btn为false
    void sig_auth_rsp_set_btn_false(std::shared_ptr<FriendInfo>);

    /*有聊天内容更新*/
    void sig_text_chat_msg(std::shared_ptr<TextChatMsg> msg);
};

#endif // TCPMGR_H
