#ifndef CHATVIEW_H
#define CHATVIEW_H
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTimer>

class ChatView : public QWidget
{
    Q_OBJECT
public:
    ChatView(QWidget *parent = Q_NULLPTR);
    //插入聊天项QWidget
    void appendChatItem(QWidget *item);                     //头插
    void prependChatItem(QWidget *item);                    //尾插
    void insertChatItem(QWidget *before, QWidget *item);    //中间插
    //移除所有聊天项
    void removeAllItem();

protected:
    //当鼠标进入或离开QScrollArea，显示或隐藏滚动条
    bool eventFilter(QObject *o, QEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    //滚动条滚动到最底部，一般来新消息时触发
    void onVScrollBarMoved(int min, int max);

private:
    void initStyleSheet();

private:
    //QWidget *m_pCenterWidget;
    QVBoxLayout *m_pVl;
    //显示出来的聊天区域
    QScrollArea *m_pScrollArea;
    bool isAppended;

};

#endif // CHATVIEW_H
