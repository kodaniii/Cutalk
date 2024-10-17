#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "defs.h"
RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    ui->passwd_lineEdit->setEchoMode(QLineEdit::Password);
    ui->confirm_pswd_lineEdit->setEchoMode(QLineEdit::Password);

    ui->msg_output_label->setProperty("state", "normal");
    repolish(ui->msg_output_label);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_verificationCode_get_Button_clicked()
{
    auto email = ui->email_edit->text();

    static QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool isMatch = regex.match(email).hasMatch();

    qDebug() << "get email = " << email
        << ", isMatch = " << isMatch;
    if(isMatch){
        //TODO send http vertify code
        showTip(isMatch, tr("验证码已发送"));
    }
    else{
        showTip(isMatch, tr("邮箱地址不正确"));
    }
}

void RegisterDialog::showTip(bool stat, QString str){
    if(stat){
        ui->msg_output_label->setProperty("state", "normal");
    }
    else{
        ui->msg_output_label->setProperty("state", "error");
    }
    ui->msg_output_label->setText(str);
    repolish(ui->msg_output_label);
}
