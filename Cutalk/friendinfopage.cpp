#include "friendinfopage.h"
#include "ui_friendinfopage.h"
#include <QDebug>
#include <QRandomGenerator>

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

    //显示性别，由于还没做性别相关参数，所以随机生成
    int randomValue = QRandomGenerator::global()->bounded(2);
    QString sex_icon = "";
    if(randomValue){
        sex_icon = ":/res/sex_male";
    }
    else {
        sex_icon = ":/res/sex_female";
    }
    QPixmap pixmap_icon(sex_icon);
    ui->sex_lb->setPixmap(pixmap_icon.scaled(ui->sex_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

}

void FriendInfoPage::on_msg_chat_clicked()
{
    qDebug() << "FriendInfoPage::on_msg_chat_clicked()";
    emit sig_jump_chat_item(_user_info);
}
