#ifndef APPLYFRIENDITEM_H
#define APPLYFRIENDITEM_H

#include <QWidget>
#include <listitembase.h>
#include "userdata.h"
#include <memory>


namespace Ui {
class ApplyFriendItem;
}

class ApplyFriendItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ApplyFriendItem(QWidget *parent = nullptr);
    ~ApplyFriendItem();

    /*把好友申请信息指针存储到_apply_info中，并设置item的参数*/
    void SetInfo(std::shared_ptr<ApplyInfo>);
    /*显示添加好友按钮或已添加Label*/
    void ShowAddBtn(bool);

    QSize sizeHint() const override {
        return QSize(250, 80);
    }

    int GetUid();

private:
    Ui::ApplyFriendItem *ui;

    std::shared_ptr<ApplyInfo> _apply_info;
    bool _added;

signals:
    /*对于该item，点击同意好友申请按钮，TCPMGR发送认证同意请求*/
    void sig_auth_friend(std::shared_ptr<ApplyInfo> apply_info);
};

#endif // APPLYFRIENDITEM_H
