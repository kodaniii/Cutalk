#include "chatuserwid.h"
#include "ui_chatuserwid.h"

ChatUserWid::ChatUserWid(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

QSize ChatUserWid::sizeHint() const
{
    return QSize(250, 70);
}

void ChatUserWid::SetInfo(QString name, QString head, QString msg, int uid)
{
    _name = name;
    _head = head;
    _msg = msg;
    _uid = uid;

    //加载图片
    QPixmap pixmap(_head);

    //设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    /*手动截断联系人界面的聊天文本，如果截断，保留前面部分并添加省略号*/
    QFontMetrics fontMetrics_name(ui->user_name_lb->font());
    QString nameText = fontMetrics_name.elidedText(_name, Qt::ElideRight, ui->user_name_lb->width());
    QFontMetrics fontMetrics_msg(ui->user_chat_lb->font());
    QString msgText = fontMetrics_msg.elidedText(_msg, Qt::ElideRight, ui->user_chat_lb->width());


    ui->user_name_lb->setText(nameText);
    ui->user_chat_lb->setText(msgText);


}


void ChatUserWid::SetInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
    // 加载图片
    QPixmap pixmap(_user_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_user_info->_name);
    ui->user_chat_lb->setText(_user_info->_last_msg);
}

void ChatUserWid::SetInfo(std::shared_ptr<FriendInfo> friend_info)
{
    qDebug() << "ChatUserWid::SetInfo()";
    _user_info = std::make_shared<UserInfo>(friend_info);
    // 加载图片
    QPixmap pixmap(_user_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_user_info->_name);
    ui->user_chat_lb->setText(_user_info->_last_msg);
    //qDebug() << "ChatUserWid::SetInfo() succ";
}

std::shared_ptr<UserInfo> ChatUserWid::GetUserInfo()
{
   return _user_info;
}

void ChatUserWid::updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs)
{
    QString last_msg = "";
    for (auto& msg : msgs) {
        last_msg = msg->_msg_content;
        _user_info->_chat_msgs.push_back(msg);
    }

    _user_info->_last_msg = last_msg;
    ui->user_chat_lb->setText(_user_info->_last_msg);
}
