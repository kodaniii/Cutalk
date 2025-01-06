#include "friendinfopage.h"
#include "ui_friendinfopage.h"
#include <QDebug>

FriendInfoPage::FriendInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendInfoPage),_user_info(nullptr)
{
    ui->setupUi(this);
    ui->msg_chat->init("normal","hover","press");
    ui->video_chat->init("normal","hover","press");
    ui->voice_chat->init("normal","hover","press");
}

FriendInfoPage::~FriendInfoPage()
{
    delete ui;
}

void FriendInfoPage::SetInfo(std::shared_ptr<FriendInfo> user_info)
{
    qDebug() << "FriendInfoPage::SetInfo(FriendInfo) FriendInfo -> UserInfo";
    _user_info = user_info;

    QPixmap pixmap(user_info->_icon);

    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->name_lb->setText(user_info->_name);
    ui->nick_lb->setText(user_info->_nick);
    ui->bak_lb->setText(user_info->_back);
}

void FriendInfoPage::on_msg_chat_clicked()
{
    qDebug() << "FriendInfoPage::on_msg_chat_clicked()";
    emit sig_jump_chat_item(_user_info);
}
