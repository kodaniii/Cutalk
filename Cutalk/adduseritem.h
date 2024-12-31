#ifndef ADDUSERITEM_H
#define ADDUSERITEM_H

#include <QWidget>
#include "listitembase.h"
#include <QMovie>

namespace Ui {
class AddUserItem;
}

class AddUserItem : public ListItemBase
{
    Q_OBJECT
public:
    explicit AddUserItem(QWidget *parent = nullptr);
    ~AddUserItem();

    QSize sizeHint() const override {
        return QSize(280, 60); // 返回自定义的尺寸
    }

    /*添加动画，等待TCP传回联系人搜索结果*/
    void loading_user_search_rsp(bool pending);
    /*取消搜索没有返回的loading状态，当用户点击其他界面时触发*/
    void set_search_loading_to_right();

private:
    Ui::AddUserItem *ui;
    QMovie *_movie;

};

#endif // ADDUSERITEM_H
