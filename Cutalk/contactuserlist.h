#ifndef CONTACTUSERLIST_H
#define CONTACTUSERLIST_H
#include <QListWidget>
#include <QEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QDebug>
#include <memory>
#include "userdata.h"
class ConUserItem;

class ContactUserList : public QListWidget
{
    Q_OBJECT
public:
    ContactUserList(QWidget* parent = nullptr);
    /*显示或隐藏新朋友请求的红点提示*/
    void ShowRedPoint(bool bshow = true);

    /*更新更多联系人项*/
    void addContactUserList();

protected:
    /*事件过滤器：滚动条的显示和隐藏*/
    bool eventFilter(QObject *watched, QEvent *event) override ;

private:
    /*初始化好友申请和联系人分组，包括好友申请和联系人请求项*/
    void initContactUserList();

public slots:
    /*列表项点击事件：处理联系人项或新朋友请求项，显示相关信息*/
    void slot_item_clicked(QListWidgetItem *item);
    /*我方主动发送好友添加申请，对方同意*/
    void slot_add_auth_firend(std::shared_ptr<FriendInfo>);
    /*我方同意其他人的好友添加申请*/
    void slot_auth_rsp(std::shared_ptr<AuthRsp>);

signals:
    /*加载更多联系人*/
    void sig_loading_contact_user();
    /*切换最右侧页面为好友申请列表*/
    void sig_switch_apply_friend_page();
    /*点击联系人，切换最右侧页面为该好友详细信息，允许编辑*/
    void sig_switch_friend_info_page(/*std::shared_ptr<UserInfo> user_info*/);

private:
    /*标记正在加载联系人数据，不允许其他操作*/
    bool _load_pending;
    /*好友申请分组下的好友申请项，只有这一个固定项，属于好友申请分组，当有好友申请时，显示红点*/
    ConUserItem *_add_friend_item;
    /*Widget对象，"联系人"分组，用于确定位置，后面的新朋友可能会插入到联系人分组后面*/
    QListWidgetItem *_groupitem;
};

#endif // CONTACTUSERLIST_H
