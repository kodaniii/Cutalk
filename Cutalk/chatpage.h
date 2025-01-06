#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QDialog>
#include <QKeyEvent>
#include "userdata.h"

namespace Ui {
class ChatPage;
}

class ChatPage : public QDialog
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();

    void SetUserInfo(std::shared_ptr<UserInfo> user_info);
    void AppendChatMsg(std::shared_ptr<TextChatData> msg);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Escape) {
            // 忽略ESC键事件，不调用QDialog::keyPressEvent(event)
            event->ignore();
        } else {
            // 对于其他按键事件，调用基类的keyPressEvent处理
            QDialog::keyPressEvent(event);
        }
    }

signals:
    //保存聊天记录到当前聊天用户recv_uid中，防止切换的时候聊天记录丢失
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData> msg);

private slots:
    void on_send_btn_clicked();

private:
    Ui::ChatPage *ui;

    std::shared_ptr<UserInfo> _user_info;
};

#endif // CHATPAGE_H
