#ifndef CHATITEMBASE_H
#define CHATITEMBASE_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include "defs.h"

class BubbleFrame;

class ChatItemBase: public QWidget
{
    Q_OBJECT
public:
    explicit ChatItemBase(ChatRole role, QWidget *parent = nullptr);

    void setUserName(const QString &name);
    void setUserIcon(const QPixmap &icon);
    //替换消息内容，聊天项重用，减少内存分配的损耗
    void setWidget(QWidget *w);

private:
    //消息发送方self/other
    ChatRole m_role;
    //显示用户名
    QLabel *m_pNameLabel;
    //显示用户图标
    QLabel *m_pIconLabel;
    //显示消息内容
    QWidget *m_pBubble;
};

#endif // CHATITEMBASE_H
