#include "chatuserlist.h"
#include <QScrollBar>
#include <QTimer>
#include <QCoreApplication>
#include "usermgr.h"

ChatUserList::ChatUserList(QWidget *parent):
    QListWidget(parent),
    _load_pending(false){

    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //安装事件过滤器
    this->viewport()->installEventFilter(this);
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *event) {
    //检查事件是否是鼠标悬浮进入或离开
    if (watched == this->viewport()) {
        //鼠标悬浮，显示滚动条
        if (event->type() == QEvent::Enter) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        //鼠标离开，隐藏滚动条
        else if (event->type() == QEvent::Leave) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    //检查事件是否是鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        //y轴上下滚动
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        //计算滚动步数
        int numSteps = numDegrees / 15;

        //设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

        //检查是否滚动到底部
        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        //int pageSize = 10; // 每页加载的联系人数量

        if (maxScrollValue - currentValue <= 0) {
            // 滚动到底部，加载新的联系人
            //qDebug() << "currentValue <= maxScrollValue, load more content";

            auto b_loaded = UserMgr::GetInstance()->IsLoadChatFin();
            //已经加载完所有聊天项，return
            if(b_loaded){
                qDebug() << "b_loaded" << (b_loaded? "true": "false");
                return true;
            }

            //正在加载，防止重复加载，return
            if(_load_pending){
                return true;
            }

            _load_pending = true;
            //发送信号通知聊天界面加载更多聊天内容
            emit sig_loading_chat_user();
        }

        return true; //停止事件传递
    }

    return QListWidget::eventFilter(watched, event);
}
