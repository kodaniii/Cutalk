#include "chatpage.h"
#include "ui_chatpage.h"
#include <QStyleOption>
#include <QPainter>

ChatPage::ChatPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatPage)
{
    ui->setupUi(this);

    //设置按钮样式
    ui->recv_btn->init("normal", "hover", "press");
    ui->send_btn->init("normal", "hover", "press");

    //设置图标样式
    ui->emoji_lb->init("normal", "hover", "press", "normal", "hover", "press");
    ui->file_lb->init("normal", "hover", "press", "normal", "hover", "press");
}

ChatPage::~ChatPage()
{
    delete ui;
}

void ChatPage::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
