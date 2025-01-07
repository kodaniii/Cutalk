#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>
#include "listitembase.h"
#include "userdata.h"

namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();

    QSize sizeHint() const override;
    void SetInfo(QString name, QString head, QString msg, int uid);
    void SetInfo(std::shared_ptr<UserInfo> user_info);
    void SetInfo(std::shared_ptr<FriendInfo> friend_info);

    std::shared_ptr<UserInfo> GetUserInfo();

    //聊天list中对应uid的小横条
    //对方发送消息，本地保存当前聊天用户_user_info的聊天记录，并更新最后一次聊天内容
    void updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs);

private:
    Ui::ChatUserWid *ui;

    QString _name;
    QString _head;
    QString _msg;
    int _uid;

    std::shared_ptr<UserInfo> _user_info;
};

#endif // CHATUSERWID_H
