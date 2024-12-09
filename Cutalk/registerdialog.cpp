#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "defs.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
    , count_down(5) {
    ui->setupUi(this);

    ui->passwd_lineEdit->setEchoMode(QLineEdit::Password);
    ui->confirm_pswd_lineEdit->setEchoMode(QLineEdit::Password);

    ui->msg_output_label->setProperty("state", "normal");
    repolish(ui->msg_output_label);

    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);

    initHttpHandlers();

    //ui->msg_output_label.clear();
    //clear()会直接清空label，等有消息弹出时，各个组件的位置会移动
    ui->msg_output_label->setText("");

    connect(ui->username_lineEdit, &QLineEdit::editingFinished, this, [this]() -> bool {
        //checkUserValid();
        if(ui->username_lineEdit->text().length() < 4){
            AddTipErr(TipErr::TIP_USER_ERR, tr("用户名长度必须大于等于4个字符"));
            return false;
        }

        //都满足, 删除对应错误
        DelTipErr(TipErr::TIP_USER_ERR);
        return true;
    });

    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this]() -> bool {
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
    });

    connect(ui->passwd_lineEdit, &QLineEdit::editingFinished, this, [this]() -> bool {
        //checkPswdValid();
        auto pswd = ui->passwd_lineEdit->text();
        auto cfm_pswd = ui->confirm_pswd_lineEdit->text();

        if(pswd.length() < 4 || pswd.length() > 15){
            AddTipErr(TipErr::TIP_PSWD_ERR, tr("密码长度必须满足4-15字符"));
            return false;
        }

        // 创建一个正则表达式对象，按照上述密码要求
        // 这个正则表达式解释：
        // ^[a-zA-Z0-9!@#$%^&*]{4,15}$ 密码长度至少4，可以是字母、数字和特定的特殊字符
        QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{4,15}$");
        bool match = regExp.match(pswd).hasMatch();
        if(!match){
            //提示字符非法
            AddTipErr(TipErr::TIP_PSWD_ERR, tr("密码不能包含非法字符"));
            return false;;
        }

        //密码部分无误
        DelTipErr(TipErr::TIP_PSWD_ERR);

        if(!cfm_pswd.isEmpty() && pswd != cfm_pswd){
            //提示密码不匹配
            AddTipErr(TipErr::TIP_PSWD_CONFIRM_ERR, tr("密码和确认密码不匹配"));
            return false;
        }

        DelTipErr(TipErr::TIP_PSWD_CONFIRM_ERR);
        return true;
    });

    connect(ui->confirm_pswd_lineEdit, &QLineEdit::editingFinished, this, [this]() -> bool {
        //checkConfirmPswdValid();
        auto pswd = ui->passwd_lineEdit->text();
        auto cfm_pswd = ui->confirm_pswd_lineEdit->text();

        if(cfm_pswd.length() < 4 || cfm_pswd.length() > 15){
            AddTipErr(TipErr::TIP_CONFIRM_PSWD_ERR, tr("确认密码长度必须满足4-15字符"));
            return false;
        }

        // 创建一个正则表达式对象，按照上述密码要求
        // 这个正则表达式解释：
        // ^[a-zA-Z0-9!@#$%^&*]{4,15}$ 密码长度至少4，可以是字母、数字和特定的特殊字符
        QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{4,15}$");
        bool match = regExp.match(pswd).hasMatch();
        if(!match){
            //提示字符非法
            AddTipErr(TipErr::TIP_CONFIRM_PSWD_ERR, tr("确认密码不能包含非法字符"));
            return false;;
        }

        DelTipErr(TipErr::TIP_CONFIRM_PSWD_ERR);

        if(pswd != cfm_pswd){
            //提示密码不匹配
            AddTipErr(TipErr::TIP_PSWD_CONFIRM_ERR, tr("密码和确认密码不匹配"));
            return false;
        }

        DelTipErr(TipErr::TIP_PSWD_CONFIRM_ERR);
        return true;
    });

    connect(ui->verificationCode_lineEdit, &QLineEdit::editingFinished, this, [this]() -> bool {
        //checkVerifyCodeValid();
        auto vcode = ui->verificationCode_lineEdit->text();
        if(vcode.isEmpty()){
            AddTipErr(TipErr::TIP_VERIFY_CODE_ERR, tr("验证码不能为空"));
            return false;
        }

        DelTipErr(TipErr::TIP_VERIFY_CODE_ERR);
        return true;
    });


    //鼠标移动到该label时，变为手状
    //已经在提升为ClickLabel类的label的构造函数中实现
    //ui->passwd_visible->setCursor(Qt::PointingHandCursor);
    //ui->confirm_pswd_visible->setCursor(Qt::PointingHandCursor);

    //初始化
    ui->passwd_visible->init("unvisible", "unvisible_hover", "",
                             "visible", "visible_hover", "");
    ui->confirm_pswd_visible->init("unvisible", "unvisible_hover", "",
                                   "visible", "visible_hover", "");

    //根据visible_label的状态，设置密码是否可见
    connect(ui->passwd_visible, &ClickLabel::clicked, this, [this](){
        auto state = ui->passwd_visible->GetCurState();
        if(state == LabelClickState::Unselected){
            ui->passwd_lineEdit->setEchoMode(QLineEdit::Password);
        }else{
            ui->passwd_lineEdit->setEchoMode(QLineEdit::Normal);
        }
        //qDebug() << "Label was clicked!";
    });

    connect(ui->confirm_pswd_visible, &ClickLabel::clicked, this, [this](){
        auto state = ui->confirm_pswd_visible->GetCurState();
        if(state == LabelClickState::Unselected){
            ui->confirm_pswd_lineEdit->setEchoMode(QLineEdit::Password);
        }else{
            ui->confirm_pswd_lineEdit->setEchoMode(QLineEdit::Normal);
        }
        //qDebug() << "Label was clicked!";
    });


    //注册页面2跳转回登陆界面的定时器
    count_down_timer = new QTimer(this);
    connect(count_down_timer, &QTimer::timeout, [this](){
        if(count_down == 1){
            count_down_timer->stop();
            emit sig_switch_login();
            return;
        }
        count_down--;
        auto str = QString("注册成功，%1 秒后跳转登录界面").arg(count_down);
        ui->page2_msg_label->setText(str);
    });
}

RegisterDialog::~RegisterDialog(){
    delete ui;
}

void RegisterDialog::ChangeRegisterDialogPage(){
    count_down_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);

    // 启动定时器，设置间隔为1000毫秒（1秒）
    count_down_timer->start(1000);
}

void RegisterDialog::AddTipErr(TipErr te, QString tips){
    tipErrs[te] = tips;
    showTip(false, tips);
}

void RegisterDialog::DelTipErr(TipErr te){
    tipErrs.remove(te);

    if(tipErrs.empty()){
        ui->msg_output_label->clear();
        return;
    }

    showTip(false, tipErrs.first());
}

bool RegisterDialog::isVaildEmail(QString email){
    static QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool isMatch = regex.match(email).hasMatch();

    qDebug() << "RegisterDialog::isVaildEmail() =" << isMatch;
    return isMatch;
}

//获取验证码button
void RegisterDialog::on_verificationCode_get_Button_clicked(){
    auto email = ui->email_edit->text();
    bool isMatch = isVaildEmail(email);

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
    //TODO register back button, is ok
    //qDebug() << "RegisterDialog::on_cancel_button_clicked()";
    count_down_timer->stop();
    emit sig_switch_login();
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

        if(err == StatusCodes::VerifyExpired){
            showTip(false, tr("验证码不存在或过期"));
            return;
        }

        if(err == StatusCodes::VerifyCodeErr){
            showTip(false, tr("验证码错误"));
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
        ChangeRegisterDialogPage();
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
    json_obj["pswd"] = xorString(ui->passwd_lineEdit->text());
    json_obj["confirm_pswd"] = xorString(ui->confirm_pswd_lineEdit->text());
    json_obj["verifycode"] = ui->verificationCode_lineEdit->text();
    //发注册请求
    HttpMgr::GetInstance()->postHttpReq(QUrl(GateServer_url_perfix + "/user_register"),
                                        json_obj, HttpReqId::REQ_REG_USER, Modules::MOD_REGISTER);
}

//注册界面Page2点击返回登录按钮
void RegisterDialog::on_return_logic_ui_button_clicked() {
    count_down_timer->stop();
    emit sig_switch_login();
}

