#ifndef APPLYFRIENDLIST_H
#define APPLYFRIENDLIST_H
#include <QListWidget>
#include <QEvent>

class ApplyFriendList: public QListWidget
{
    Q_OBJECT
public:
    ApplyFriendList(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    /*如果点击了任意区域，就会取消搜索栏的显示*/
    void sig_show_search(bool);
};

#endif // APPLYFRIENDLIST_H
