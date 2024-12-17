#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "defs.h"
#include <QKeyEvent>

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();

    void addChatUserList();

protected:
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

    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;

    bool _b_loading;
};

#endif // CHATDIALOG_H
