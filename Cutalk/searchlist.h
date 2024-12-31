#ifndef SEARCHLIST_H
#define SEARCHLIST_H
#include <QListWidget>
#include <QWheelEvent>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>
#include <QDialog>
#include <memory>
#include "userdata.h"
//#include "loadingdlg.h"
#include "adduseritem.h"

class SearchList: public QListWidget
{
    Q_OBJECT
public:
    SearchList(QWidget *parent = nullptr);
    //关闭查找对话框
    void CloseFindDlg();
    void SetSearchEdit(QWidget* edit);

protected:
    //鼠标进入显示滚动条，离开不显示滚动条
    bool eventFilter(QObject *watched, QEvent *event) override {
        //检查事件是否是鼠标悬浮进入或离开
        if (watched == this->viewport()) {
            if (event->type() == QEvent::Enter) {
                // 鼠标悬浮，显示滚动条
                this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            } else if (event->type() == QEvent::Leave) {
                // 鼠标离开，隐藏滚动条
                this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            }
        }

        // 检查事件是否是鼠标滚轮事件
        if (watched == this->viewport() && event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            int numDegrees = wheelEvent->angleDelta().y() / 8;
            int numSteps = numDegrees / 15; // 计算滚动步数

            // 设置滚动幅度
            this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

            return true; // 停止事件传递
        }

        return QListWidget::eventFilter(watched, event);
    }

private:
    //显示loading
    //（后期可能去掉）不去掉了，重新写了逻辑
    void waitPending(bool pending = true);
    //是否有服务器回包，如果没有必须阻塞等待，禁止用户交互
    bool _send_pending;
    void initTipItem();
    std::shared_ptr<QDialog> _find_dlg;
    //Search_edit（位于chatdialog），方便获取search_edit的文本内容
    QWidget* _search_edit;
    //LoadingDlg *_loadingDialog;

    //维护查找uid/name的项，便于更改loading git图标
    AddUserItem* _add_user_item;

private slots:
    void slot_item_clicked(QListWidgetItem *item);
    void slot_user_search(bool, std::shared_ptr<SearchInfo>);
signals:

};

#endif
