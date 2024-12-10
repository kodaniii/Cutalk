#include "resetdialog.h"
#include "ui_resetdialog.h"
#include <QDebug>
#include <QRegularExpression>
#include "httpmgr.h"

ResetDialog::ResetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResetDialog) {
    ui->setupUi(this);

    connect(ui->username_edit, &QLineEdit::editingFinished, this, [this](){
        //checkUserValid();
        qDebug() << "checkUserValid() ret" << checkUserValid();
    });

    connect(ui->pwd_edit, &QLineEdit::editingFinished, this, [this](){
        //checkPasswdValid();
        qDebug() << "checkPasswdValid() ret" << checkPasswdValid();
    });

    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){
        //checkEmailValid();
        qDebug() << "checkEmailValid() ret" << checkEmailValid();
    });

    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this](){
        //checkVerifyCodeValid();
        qDebug() << "checkVerifyCodeValid() ret" << checkVerifyCodeValid();
    });

    //连接reset相关信号和注册处理回调
    initHandlers();

    ui->msg_output_label->setText("");

    //来自HttpMgr::slot_http_finish发来的完成信号
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reset_mod_finish,
            this, &ResetDialog::slot_reset_mod_finish);


    //初始化
    ui->passwd_visible->init("unvisible", "unvisible_hover", "",
                             "visible", "visible_hover", "");
    ui->pwd_edit->setEchoMode(QLineEdit::Password);

    //根据visible_label的状态，设置密码是否可见
    connect(ui->passwd_visible, &ClickLabel::clicked, this, [this](){
        auto state = ui->passwd_visible->GetCurState();
        if(state == LabelClickState::Unselected){
            ui->pwd_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->pwd_edit->setEchoMode(QLineEdit::Normal);
        }
        //qDebug() << "Label was clicked!";
    });
}


ResetDialog::~ResetDialog()
{
    delete ui;
}

void ResetDialog::on_return_btn_clicked()
{
    qDebug() << "ResetDialog::on_return_btn_clicked()";
    emit sig_switch_login();
}

//获取验证码button
void ResetDialog::on_verify_btn_clicked()
{
    qDebug() << "ResetDialog::on_verify_btn_clicked()";
    auto email = ui->email_edit->text();
    auto isMatch = checkEmailValid();

    if(isMatch){
        //TODO send http vertify code
        QJsonObject json_obj;
        json_obj["email"] = email;
        //QString _url = "http://localhost:8080/get_verifycode";
        QString _url = GateServer_url_perfix + "/get_verifycode";
        qDebug() << "RegisterDialog::on_verificationCode_get_Button_clicked postHttpReq Qurl =" << _url;
        HttpMgr::GetInstance()->postHttpReq(QUrl(_url), json_obj,
                                            HttpReqId::REQ_GET_VERIFY_CODE, Modules::MOD_RESET);

        showTip(isMatch, tr("正在发送验证码"));
    }
    else{
        showTip(isMatch, tr("邮箱地址不正确"));
    }
}

void ResetDialog::slot_reset_mod_finish(HttpReqId id, QString res, StatusCodes err)
{
    if(err != StatusCodes::SUCCESS){
        showTip(false, tr("GateServer服务连接失败"));
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(false, tr("json解析错误"));
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(false, tr("json解析错误"));
        return;
    }

    qDebug() << "RegisterDialog::slot_reg_mod_finish _handlers req_id =" << id;

    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());

    return;
}

bool ResetDialog::checkUserValid()
{
    auto user = ui->username_edit->text();

    QRegularExpression regex(R"([@\.])");
    bool match = regex.match(ui->username_edit->text()).hasMatch();
    if(match){
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能包含@和."));
        return false;
    }

    if(user.length() < 4){
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名长度必须大于等于4个字符"));
        return false;
    }

    //都满足, 删除对应错误
    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool ResetDialog::checkPasswdValid()
{
    auto pswd = ui->pwd_edit->text();

    if(pswd.length() < 4 || pswd.length() > 15){
        AddTipErr(TipErr::TIP_PSWD_ERR, tr("密码长度必须满足4-15字符"));
        return false;
    }

    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{4,15}$");
    bool match = regExp.match(pswd).hasMatch();
    if(!match){
        //提示字符非法
        AddTipErr(TipErr::TIP_PSWD_ERR, tr("密码不能包含非法字符"));
        return false;;
    }

    //密码部分无误
    DelTipErr(TipErr::TIP_PSWD_ERR);
    return true;
}



bool ResetDialog::checkEmailValid()
{
    //checkEmailValid();
    auto email = ui->email_edit->text();
    //邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(!match){
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }

    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool ResetDialog::checkVerifyCodeValid()
{
    auto vcode = ui->verify_edit->text();
    if(vcode.isEmpty()){
        AddTipErr(TipErr::TIP_VERIFY_CODE_ERR, tr("验证码不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_VERIFY_CODE_ERR);
    return true;
}

void ResetDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(false, tips);
}

void ResetDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
      ui->msg_output_label->clear();
      return;
    }

    showTip(false, _tip_errs.first());
}

void ResetDialog::initHandlers()
{
    //获取验证码回包逻辑
    _handlers.insert(HttpReqId::REQ_GET_VERIFY_CODE, [this](const QJsonObject& jsonObj){
        int err = jsonObj["error"].toInt();
        qDebug() << "ResetDialog::initHandlers() REQ_GET_VERIFY_CODE" << err;
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

    //重置密码回包逻辑
    _handlers.insert(HttpReqId::REQ_RESET_PSWD, [this](QJsonObject jsonObj){
        int err = jsonObj["error"].toInt();
        qDebug() << "ResetDialog::initHandlers() REQ_RESET_PSWD" << err;

        if(err == StatusCodes::EmailNotRegistered){
            showTip(false, tr("该邮箱还没有被注册，不允许重置密码"));
            return;
        }

        if(err == StatusCodes::UsernameCannotUse){
            showTip(false, tr("用户名已经被其他用户注册"));
            return;
        }

        if(err == StatusCodes::VerifyExpired){
            showTip(false, tr("验证码不存在或过期"));
            return;
        }

        if(err == StatusCodes::VerifyCodeErr){
            showTip(false, tr("验证码错误"));
            return;
        }

        if(err == StatusCodes::ResetUpdateFailed){
            showTip(false, tr("重置用户名和密码失败"));
            return;
        }

        if(err != StatusCodes::SUCCESS){
            showTip(false, tr("不可预知的错误"));
            return;
        }
        auto email = jsonObj["email"].toString();
        auto user = jsonObj["user"].toString();
        auto pswd = xorString(jsonObj["pswd"].toString());
        showTip(true, tr("重置成功, 点击返回登录"));
        qDebug() << "[RESET] email" << email ;
        qDebug() << "[RESET] uid" << jsonObj["uid"].toString();
        qDebug() << "[RESET] user name" << user;
        qDebug() << "[RESET] user pswd" << pswd;
    });
}

void ResetDialog::showTip(bool stat, QString str)
{
    if(stat){
        ui->msg_output_label->setProperty("state", "normal");
    }
    else{
        ui->msg_output_label->setProperty("state", "error");
    }
    ui->msg_output_label->setText(str);
    repolish(ui->msg_output_label);
}

void ResetDialog::on_sure_btn_clicked()
{
    qDebug() << "ResetDialog::on_sure_btn_clicked()";
    bool valid = checkUserValid();
    if(!valid){
        return;
    }

    valid = checkEmailValid();
    if(!valid){
        return;
    }

    valid = checkPasswdValid();
    if(!valid){
        return;
    }

    valid = checkVerifyCodeValid();
    if(!valid){
        return;
    }

    qDebug() << "ResetDialog::on_sure_btn_clicked() valid" << valid;

    showTip(true, "重置请求已发送");
    //发送http重置请求
    QJsonObject json_obj;
    json_obj["email"] = ui->email_edit->text();
    json_obj["user"] = ui->username_edit->text();
    json_obj["pswd"] = xorString(ui->pwd_edit->text());
    json_obj["verifycode"] = ui->verify_edit->text();
    HttpMgr::GetInstance()->postHttpReq(QUrl(GateServer_url_perfix + "/reset_user_and_passwd"),
                 json_obj, HttpReqId::REQ_RESET_PSWD, Modules::MOD_RESET);
}
