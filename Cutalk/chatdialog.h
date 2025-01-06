#ifndef CHATDIALOG_H
#define CHATDIALOG_H


#include <QDialog>
#include "defs.h"
#include <QKeyEvent>
#include "statewidget.h"
#include "userdata.h"
#include <QListWidgetItem>

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();


protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Escape) {
            // 忽略ESC键事件，不调用QDialog::keyPressEvent(event)
            event->ignore();
        } else {
            // 对于其他按键事件，调用基类的keyPressEvent处理
            QDialog::keyPressEvent(event);
        }
    }

private:
    void ShowSearch(bool b_search = false);

    void addChatUserList();

    void addSideGroup(StateWidget *sw);
    void clearSideState(StateWidget *sw);

    void handleGlobalMousePress(QMouseEvent *event);

    //选中其中一个聊天项
    void SetSelectChatItem(int uid = 0);
    void SetSelectChatPage(int uid = 0);

    QList<StateWidget*> _side_list;

    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;

    bool _b_loading;

    //保存uid的已添加聊天项
    QMap<int, QListWidgetItem*> _chat_items_added;

    //当前正在聊天的uid
    int _cur_chat_uid;

    /*加载下一页聊天列表*/
    void loadMoreChatUser();
    void loadMoreConUser();

    //记录上一次界面的状态，记录的是最右面的界面，即是好友申请界面还是某个好友的名片friendinfo信息
    QWidget* _last_widget;

private slots:
    void slot_loading_chat_user();

    void slot_side_chat();
    void slot_side_contact();

    //search_edit更新，切换到搜索界面
    void slot_search_change(const QString &str);

    //加载更多联系人->联系人项
    void slot_loading_contact_user();

    /*清空搜索框*/
    void slot_show_search(bool);

    /*通知添加好友，有来自其他Client的好友申请*/
    void slot_notify_apply_friend(std::shared_ptr<AddFriendApply> apply);

    //我方主动发送好友添加申请，对方同意后的回包处理，刷新联系人和聊天界面，这里处理聊天
    void slot_add_auth_friend(std::shared_ptr<FriendInfo> auth_info);

    //我方同意其他人的好友添加申请后，我方需要刷新联系人和聊天界面，这里处理聊天
    void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);

    //搜索用户，如果搜索的用户是自己的好友，触发slot，跳转到该用户的聊天item
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);

    //点击好友item，切换到对应好友信息页面
    void slot_friend_info_page(std::shared_ptr<FriendInfo> user_info);

    //点击联系人->好友申请，跳转到好友申请页面
    void slot_switch_apply_friend_page();

    //好友信息界面点击发送消息按钮，跳转到聊天界面
    void slot_jump_chat_item_from_infopage(std::shared_ptr<FriendInfo> user_info);
};

#endif // CHATDIALOG_H
