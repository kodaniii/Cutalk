#include "chatuserwid.h"
#include "ui_chatuserwid.h"

ChatUserWid::ChatUserWid(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

QSize ChatUserWid::sizeHint() const
{
    return QSize(250, 70);
}

void ChatUserWid::SetInfo(QString name, QString head, QString msg)
{
    _name = name;
    _head = head;
    _msg = msg;
    //加载图片
    QPixmap pixmap(_head);

    //设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    /*手动截断联系人界面的聊天文本，如果截断，保留前面部分并添加省略号*/
    QFontMetrics fontMetrics_name(ui->user_name_lb->font());
    QString nameText = fontMetrics_name.elidedText(_name, Qt::ElideRight, ui->user_name_lb->width());
    QFontMetrics fontMetrics_msg(ui->user_chat_lb->font());
    QString msgText = fontMetrics_msg.elidedText(_msg, Qt::ElideRight, ui->user_chat_lb->width());


    ui->user_name_lb->setText(nameText);
    ui->user_chat_lb->setText(msgText);


}
