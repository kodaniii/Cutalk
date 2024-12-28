#ifndef APPLYFRIENDPAGE_H
#define APPLYFRIENDPAGE_H

#include <QWidget>
#include "userdata.h"
#include <memory>
#include <QJsonArray>
#include <unordered_map>
#include "applyfrienditem.h"

namespace Ui {
class ApplyFriendPage;
}

class ApplyFriendPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    ~ApplyFriendPage();

    /*添加一个新的好友申请，显示到界面上*/
    void AddNewApply(std::shared_ptr<AddFriendApply> apply);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::ApplyFriendPage *ui;

    /*获取好友申请列表，并显示每个ApplyFriendItem项*/
    void initApplyList();

    /*哈希表，用来存储未认证的好友申请项*/
    std::unordered_map<int, ApplyFriendItem*> _unauth_items;

public slots:
    /*好友申请认证响应，从_unauth_items中删除对应项，并将item的添加好友按钮变更为已添加字样*/
    void slot_auth_rsp(std::shared_ptr<AuthRsp>);

signals:
    /*搜索用户，显示搜索界面*/
    void sig_show_search(bool);
};

#endif // APPLYFRIENDPAGE_H
