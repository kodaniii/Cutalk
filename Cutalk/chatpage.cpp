#include "chatpage.h"
#include "ui_chatpage.h"
#include <QStyleOption>
#include <QPainter>
#include "chatitembase.h"
#include "textbubble.h"
#include "picturebubble.h"
#include "usermgr.h"
#include <QJsonDocument>
#include "tcpmgr.h"

ChatPage::ChatPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatPage)
{
    ui->setupUi(this);

    //设置按钮样式
    //ui->recv_btn->init("normal", "hover", "press");
    ui->send_btn->init("normal", "hover", "press");

    //设置图标样式
    ui->emoji_lb->init("normal", "hover", "press", "normal", "hover", "press");
    ui->file_lb->init("normal", "hover", "press", "normal", "hover", "press");
}

ChatPage::~ChatPage()
{
    delete ui;
}

void ChatPage::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


void ChatPage::SetUserInfo(std::shared_ptr<UserInfo> user_info){
    qDebug() << "ChatPage::SetUserInfo()";
    _user_info = user_info;
    qDebug() << " -> user_info uid" << _user_info->_uid
             << "name" << _user_info->_name
             << "nick" << _user_info->_nick
             << "sex" << _user_info->_sex
             << "icon" << _user_info->_icon;
    //设置聊天用户的name
    //TODO 如果有备注改为备注
    ui->title_lb->setText(_user_info->_name);
    //清除所有聊天项（除最后一个占位项）
    ui->chat_data_list->removeAllItem();
    for(auto& msg: user_info->_chat_msgs){
        AppendChatMsg(msg);
    }
}

void ChatPage::AppendChatMsg(std::shared_ptr<TextChatData> msg)
{
    auto self_info = UserMgr::GetInstance()->GetUserInfo();
    ChatRole role;
    //添加聊天显示
    if (msg->_send_uid == self_info->_uid) {
        role = ChatRole::Self;
        ChatItemBase* pChatItem = new ChatItemBase(role);

        pChatItem->setUserName(self_info->_name);
        pChatItem->setUserIcon(QPixmap(self_info->_icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chat_data_list->appendChatItem(pChatItem);
    }
    else {
        role = ChatRole::Other;
        ChatItemBase* pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::GetInstance()->GetFriendById(msg->_send_uid);
        if (friend_info == nullptr) {
            delete pChatItem;
            return;
        }
        pChatItem->setUserName(friend_info->_name);
        pChatItem->setUserIcon(QPixmap(friend_info->_icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chat_data_list->appendChatItem(pChatItem);
    }
}

void ChatPage::on_send_btn_clicked()
{
    qDebug() << "ChatPage::on_send_btn_clicked()";

    if(_user_info == nullptr){
        qDebug() << "_user_info == nullptr, return...";
    }

    auto user_info = UserMgr::GetInstance()->GetUserInfo();
    auto pTextEdit = ui->chat_edit;
    ChatRole role = ChatRole::Self;
    //后面可能会改成昵称nick
    QString userName = user_info->_name;    //QStringLiteral("user");
    QString userIcon = user_info->_icon;    //":/res/head/head_1.jpg";

    qDebug() << " -> chat msg send_user name" << userName << ", icon" << userIcon;
    //点击item时，将chatpage的_user_info保存为recv方
    qDebug() << " -> chat msg recv_user name" << _user_info->_name << ", icon" << _user_info->_icon;

    //将单次发送的消息拆分成多个文本、图片
    const QVector<MsgInfo>& msgList = pTextEdit->getMsgList();
    QJsonObject textObj;
    QJsonArray textArray;

    int txt_size = 0;
    //将单次发送的消息发送，由于可能被图片切分成多个部分，每一个部分单独发送
    qDebug() << "Get msgList.size()" << msgList.size();
    for (int i = 0; i < msgList.size(); ++i)
    {
        //消息内容长度超过1K就跳过
        if(msgList[i].content.length() > 1024){
            continue;
        }

        QString type = msgList[i].msgFlag;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget *pBubble = nullptr;

        if(type == "text")
        {
            //生成随机id，标识消息id
            //以后可能拓展已读功能
            QUuid uuid = QUuid::createUuid();
            QString uuidString = uuid.toString();

            pBubble = new TextBubble(role, msgList[i].content);
            //累积到超过1KB就发送一次
            if(txt_size + msgList[i].content.length() > 1024){
                textObj["send_uid"] = user_info->_uid;
                textObj["send_name"] = user_info->_name;
                textObj["recv_uid"] = _user_info->_uid;
                textObj["recv_name"] = _user_info->_name;
                textObj["text_array"] = textArray;  //context + msgid
                QJsonDocument doc(textObj);
                QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
                //重置size
                txt_size = 0;
                textArray = QJsonArray();
                textObj = QJsonObject();
                //发送tcp请求给chat server
                emit TcpMgr::GetInstance()->sig_tcp_send_data(ReqId::REQ_TEXT_CHAT_MSG_REQ, jsonData);
            }

            //将bubble和uid绑定，未来可以根据rsp包实现消息已读的识别
            //_bubble_map[uuidString] = pBubble;
            txt_size += msgList[i].content.length();
            QJsonObject obj;
            QByteArray utf8Message = msgList[i].content.toUtf8();
            obj["content"] = QString::fromUtf8(utf8Message);
            obj["msgid"] = uuidString;
            textArray.append(obj);
            //TextChatData: msgid, context_string, send_uid, recv_uid
            auto txt_msg = std::make_shared<TextChatData>(uuidString, obj["content"].toString(),
                                                          user_info->_uid, _user_info->_uid);
            //保存聊天记录到当前聊天用户recv_uid中，防止切换的时候聊天记录丢失
            emit sig_append_send_chat_msg(txt_msg);
        }
        else if(type == "image")
        {
            pBubble = new PictureBubble(QPixmap(msgList[i].content) , role);
        }
        else if(type == "file")
        {

        }

        //每一条被拆分的消息都做界面上的显示
        if(pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            ui->chat_data_list->appendChatItem(pChatItem);
        }

    }

    qDebug() << "textArray" << textArray;

    //TCP ChatServer
    textObj["text_array"] = textArray;
    textObj["send_uid"] = user_info->_uid;
    textObj["recv_uid"] = _user_info->_uid;
    QJsonDocument doc(textObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    //发送后重置
    txt_size = 0;
    textArray = QJsonArray();
    textObj = QJsonObject();
    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_tcp_send_data(ReqId::REQ_TEXT_CHAT_MSG_REQ, jsonData);
}

