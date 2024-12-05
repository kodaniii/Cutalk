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

//获取验证码button
void RegisterDialog::on_verificationCode_get_Button_clicked()
{
    auto email = ui->email_edit->text();

    static QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool isMatch = regex.match(email).hasMatch();

    qDebug() << "get email =" << email << ", isMatch =" << isMatch;

    //发送postHttpReq，包含email地址
    if(isMatch){
        //TODO send http vertify code
        QJsonObject json_obj;
        json_obj["email"] = email;
        //QString _url = "http://localhost:8080/get_verifycode";
        QString _url = GateServer_url_perfix + "/get_verifycode";
        qDebug() << "RegisterDialog::on_verificationCode_get_Button_clicked postHttpReq Qurl =" << _url;
        HttpMgr::GetInstance()->postHttpReq(QUrl(_url), json_obj, HttpReqId::REQ_GET_VERIFY_CODE, Modules::MOD_REGISTER);

        showTip(isMatch, tr("正在发送验证码"));
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

//验证码获取button，处理从GateServer返回的json
void RegisterDialog::slot_reg_mod_finish(HttpReqId req_id, QString res, StatusCodes statusCode) {
    if(statusCode != StatusCodes::SUCCESS){
        showTip(false, tr("GateServer服务连接失败"));
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

    qDebug() << "RegisterDialog::slot_reg_mod_finish _handlers req_id ="
             << req_id;

    //GateServer连接成功，调用不同req的处理逻辑
    //相关req处理逻辑在RegisterDialog::initHttpHandlers()中注册
    _handlers[req_id](jsonDoc.object());
    return;
}

//HttpReqId处理逻辑
void RegisterDialog::initHttpHandlers()
{
    //GET_VERIFY_CODE request
    //从GateServer拿到json数据，json["error"]记录的是GateServer和VerifyServer连接情况
    _handlers.insert(HttpReqId::REQ_GET_VERIFY_CODE, [this](const QJsonObject& jsonObj){
        int err = jsonObj["error"].toInt();
        qDebug() << "RegisterDialog::initHttpHandlers() REQ_GET_VERIFY_CODE" << err;
        if(err == StatusCodes::RPCFailed){
            showTip(false, tr("VerifyServer服务连接失败"));
            return;
        }

        if(err != StatusCodes::SUCCESS){
            showTip(false, tr("不可预知的错误"));
            return;
        }

        auto email = jsonObj["email"].toString();
        showTip(true, tr("验证码发送成功"));
        qDebug() << "send verifycode to email success" << email;
    });

    //REQ_REG_USER
    //从GateServer拿到json回包
    _handlers.insert(HttpReqId::REQ_REG_USER, [this](const QJsonObject& jsonObj){
        int err = jsonObj["error"].toInt();
        qDebug() << "RegisterDialog::initHttpHandlers() REQ_REG_USER" << err;
        if(err == StatusCodes::UserExist){
            showTip(false, tr("用户名已经被注册过"));
            return;
        }

        if(err == StatusCodes::VerifyCodeErr){
            showTip(false, tr("验证码错误"));
            return;
        }

        if(err == StatusCodes::VerifyExpired){
            showTip(false, tr("验证码不存在或过期"));
            return;
        }

        if(err == StatusCodes::PasswdErr){
            showTip(false, tr("密码和确认密码不匹配"));
            return;
        }

        if(err != StatusCodes::SUCCESS){
            showTip(false, tr("不可预知的错误"));
            return;
        }

        auto email = jsonObj["email"].toString();
        //TODO Register逻辑, is ok
        auto uid = jsonObj["uid"].toString();
        showTip(true, tr("注册成功"));
        qDebug() << "regist success, email =" << email << ", uuid =" << uid;
    });
}

//点击注册按钮
void RegisterDialog::on_register_button_clicked()
{
    if(ui->username_lineEdit->text().isEmpty()){
        showTip(false, tr("用户名不能为空"));
        return;
    }

    if(ui->username_lineEdit->text().length() < 4){
        showTip(false, tr("用户名长度必须大于等于4个字符"));
        return;
    }

    if(ui->passwd_lineEdit->text().isEmpty()){
        showTip(false, tr("密码不能为空"));
        return;
    }

    if(ui->passwd_lineEdit->text().length() < 4){
        showTip(false, tr("密码长度必须大于等于4个字符"));
        return;
    }

    if(ui->confirm_pswd_lineEdit->text().isEmpty()){
        showTip(false, tr("确认密码不能为空"));
        return;
    }

    if(ui->confirm_pswd_lineEdit->text().length() < 4){
        showTip(false, tr("确认密码长度必须大于等于4个字符"));
        return;
    }

    if(ui->passwd_lineEdit->text() != ui->confirm_pswd_lineEdit->text()){
        showTip(false, tr("密码和确认密码不匹配"));
        return;
    }

    if(ui->email_edit->text().isEmpty()){
        showTip(false, tr("邮箱不能为空"));
        return;
    }

    if(ui->verificationCode_lineEdit->text().isEmpty()){
        showTip(false, tr("验证码不能为空"));
        return;
    }

    if(ui->verificationCode_lineEdit->text().length() != 4){
        showTip(false, tr("验证码长度必须为4个字符"));
        return;
    }

    QJsonObject json_obj;
    json_obj["user"] = ui->username_lineEdit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["pswd"] = ui->passwd_lineEdit->text();
    json_obj["confirm_pswd"] = ui->confirm_pswd_lineEdit->text();
    json_obj["verifycode"] = ui->verificationCode_lineEdit->text();
    //发注册请求
    HttpMgr::GetInstance()->postHttpReq(QUrl(GateServer_url_perfix + "/user_register"),
                                        json_obj, HttpReqId::REQ_REG_USER, Modules::MOD_REGISTER);
}

