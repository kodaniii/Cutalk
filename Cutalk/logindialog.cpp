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

}

LoginDialog::~LoginDialog() {
    qDebug() << "LoginDialog::~LoginDialog()";
    delete ui;
}

void LoginDialog::slot_forget_passwd() {
    qDebug() << "LoginDialog::slot_forget_passwd()";
    emit sig_switch_reset();
}
