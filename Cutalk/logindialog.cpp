#include "logindialog.h"
#include "ui_logindialog.h"
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog) {
    ui->setupUi(this);
    ui->passwd_edit->setEchoMode(QLineEdit::Password);
    connect(ui->register_button, &QPushButton::clicked, this, &LoginDialog::sig_switch_register);

    //forget_passwd_label也是有六个state
    ui->forget_passwd_label->init("normal", "normal_hover", "",
                                  "selected", "selected_hover", "");
    connect(ui->forget_passwd_label, &ClickLabel::clicked, this, &LoginDialog::slot_forget_passwd);


    //初始化
    ui->passwd_visible->init("unvisible", "unvisible_hover", "",
                             "visible", "visible_hover", "");
    ui->passwd_edit->setEchoMode(QLineEdit::Password);

    //根据visible_label的状态，设置密码是否可见
    connect(ui->passwd_visible, &ClickLabel::clicked, this, [this](){
        auto state = ui->passwd_visible->GetCurState();
        if(state == LabelClickState::Unselected){
            ui->passwd_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->passwd_edit->setEchoMode(QLineEdit::Normal);
        }
        //qDebug() << "Label was clicked!";
    });
}

LoginDialog::~LoginDialog() {
    qDebug() << "LoginDialog::~LoginDialog()";
    delete ui;
}

void LoginDialog::slot_forget_passwd() {
    qDebug() << "LoginDialog::slot_forget_passwd()";
    emit sig_switch_reset();
}
