#ifndef CONUSERITEM_H
#define CONUSERITEM_H

#include <QWidget>
#include "listitembase.h"
#include "userdata.h"

namespace Ui {
class ConUserItem;
}

class ConUserItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ConUserItem(QWidget *parent = nullptr);
    ~ConUserItem();

    /*返回默认尺寸*/
    QSize sizeHint() const override;
    void SetInfo(std::shared_ptr<FriendInfo> friend_info);
    void SetInfo(std::shared_ptr<AuthInfo> auth_info);
    void SetInfo(std::shared_ptr<AuthRsp> auth_rsp);
    void SetInfo(int uid, QString name, QString icon);
    void ShowRedPoint(bool show = false);
    std::shared_ptr<FriendInfo> GetInfo();

private:
    Ui::ConUserItem *ui;

    std::shared_ptr<FriendInfo> _info;
};

#endif // CONUSERITEM_H
