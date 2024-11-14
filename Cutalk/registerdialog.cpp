#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "defs.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    ui->passwd_lineEdit->setEchoMode(QLineEdit::Password);
    ui->confirm_pswd_lineEdit->setEchoMode(QLineEdit::Password);

    ui->msg_output_label->setProperty("state", "normal");
    repolish(ui->msg_output_label);

    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);

    initHttpHandlers();
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

//验证码获取button
void RegisterDialog::on_verificationCode_get_Button_clicked()
{
    auto email = ui->email_edit->text();

    static QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool isMatch = regex.match(email).hasMatch();

    qDebug() << "get email = " << email << ", isMatch = " << isMatch;
    if(isMatch){
        //TODO send http vertify code
        QJsonObject json_obj;
        json_obj["email"] = email;
        //QString _url = "http://localhost:8080/get_verifycode";
        QString _url = GateServer_url_perfix + "/get_verifycode";
        HttpMgr::GetInstance()->postHttpReq(QUrl(_url), json_obj, HttpReqId::REQ_GET_VERIFY_CODE, Modules::MOD_REGISTER);

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

void RegisterDialog::on_cancel_button_clicked()
{
    //TODO register back button
}

void RegisterDialog::slot_reg_mod_finish(HttpReqId req_id, QString res, StatusCodes statusCode) {
    if(statusCode != StatusCodes::SUCCESS){
        showTip(false, tr("网络请求错误"));
        return;
    }

    //json parser
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isNull()){
        showTip(false, tr("JSON解析错误"));
        return;
    }

    if(!jsonDoc.isObject()){
        showTip(false, tr("JSON解析错误"));
        return;
    }

    _handlers[req_id](jsonDoc.object());
    return;
}

void RegisterDialog::initHttpHandlers()
{
    //GET_VERIFY_CODE request
    _handlers.insert(HttpReqId::REQ_GET_VERIFY_CODE, [this](const QJsonObject& jsonObj){
        int err = jsonObj["error"].toInt();
        if(err != StatusCodes::SUCCESS){
            showTip(false, tr("JSON解析错误"));
        }

        auto email = jsonObj["email"].toString();
        showTip(true, tr("验证码发送成功"));
        qDebug() << "email " << email;
    });

}
