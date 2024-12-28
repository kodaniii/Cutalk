#ifndef CHATDIALOG_H
#define CHATDIALOG_H


#include <QDialog>
#include "defs.h"
#include <QKeyEvent>
#include "statewidget.h"

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

    QList<StateWidget*> _side_list;

    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;

    bool _b_loading;

private slots:
    void slot_loading_chat_user();

    void slot_side_chat();
    void slot_side_contact();

    //search_edit更新，切换到搜索界面
    void slot_search_change(const QString &str);

    //加载更多联系人->联系人项
    void slot_loading_contact_user();
};

#endif // CHATDIALOG_H
