#include "conuseritem.h"
#include "ui_conuseritem.h"

ConUserItem::ConUserItem(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ConUserItem)
{
    ui->setupUi(this);
    //设置itemle类型为联系人用户
    SetItemType(ListItemType::CONTACT_USER_ITEM);
    //小红点显示提升到顶部（Z轴）
    ui->red_point->raise();
    ShowRedPoint(false);
}

ConUserItem::~ConUserItem()
{
    delete ui;
}

//返回默认尺寸
QSize ConUserItem::sizeHint() const
{
    return QSize(250, 70);
}

void ConUserItem::SetInfo(std::shared_ptr<FriendInfo> friend_info)
{
    //_info = std::make_shared<UserInfo>(friend_info);
    _info = friend_info;
    // 加载图片
    QPixmap pixmap(_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetInfo(std::shared_ptr<AuthInfo> auth_info)
{
    _info = std::make_shared<FriendInfo>(auth_info);
    // 加载图片
    QPixmap pixmap(_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetInfo(int uid, QString name, QString icon)
{
    //_info = std::make_shared<UserInfo>(uid, name, icon);
    _info = std::make_shared<FriendInfo>(uid, name, "", icon, 0, "", "", "");

    // 加载图片
    QPixmap pixmap(_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetInfo(std::shared_ptr<AuthRsp> auth_rsp){
    //_info = std::make_shared<UserInfo>(auth_rsp);
    _info = std::make_shared<FriendInfo>(auth_rsp);

    // 加载图片
    QPixmap pixmap(_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::ShowRedPoint(bool show)
{
    if(show){
        ui->red_point->show();
    }else{
        ui->red_point->hide();
    }

}

std::shared_ptr<FriendInfo> ConUserItem::GetInfo()
{
    qDebug() << "ConUserItem::GetInfo()";
    qDebug() << " -> _info uid" << _info->_uid
             << "name" << _info->_name
             << "desc" << _info->_desc
             << "nick" << _info->_nick
             << "sex" << _info->_sex
             << "icon" << _info->_icon
             << "back" << _info->_back;
    return _info;
}
