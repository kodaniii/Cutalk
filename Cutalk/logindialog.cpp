#include "logindialog.h"
#include "ui_logindialog.h"
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include "httpmgr.h"
#include "tcpmgr.h"

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

    ui->msg_output_label->setProperty("state", "normal");
    repolish(ui->msg_output_label);
    ui->msg_output_label->setText("");

    //初始化
    ui->passwd_visible->init("unvisible", "unvisible_hover", "unvisible_hover",
                             "visible", "visible_hover", "visible_hover");
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

    initHead();
    initHandlers();

    //注册edit输入完成后的错误提示
    connect(ui->username_edit, &QLineEdit::editingFinished, this, [this](){
        //checkUserValid();
        qDebug() << "checkUserValid() ret" << checkUserValid();
    });

    connect(ui->passwd_edit, &QLineEdit::editingFinished, this, [this](){
        //checkPasswdValid();
        qDebug() << "checkPswdValid() ret" << checkPswdValid();
    });

    //注册登录回包信号
    //点击登录，发送登录请求，收到了回包信号
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish,
            this, &LoginDialog::slot_login_mod_finish);

    //tcp长连接聊天服务器
    connect(this, &LoginDialog::sig_tcp_connect, TcpMgr::GetInstance().get(), &TcpMgr::slot_tcp_connect);

    //tcp长连接完成
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_tcp_conn_fin, this, &LoginDialog::slot_tcp_conn_fin);

    //登录失败
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_login_failed, this, &LoginDialog::slot_login_failed);

}

LoginDialog::~LoginDialog() {
    qDebug() << "LoginDialog::~LoginDialog()";
    delete ui;
}

void LoginDialog::initHandlers()
{
    //用户登录回包逻辑
    _handlers.insert(ReqId::REQ_USER_LOGIN, [this](const QJsonObject& jsonObj){
        int err = jsonObj["error"].toInt();
        qDebug() << "LoginDialog::initHandlers() REQ_USER_LOGIN" << err;

        if(err == StatusCodes::LoginFailed){
            showTip(false, tr("用户名或密码错误"));
            enableBtn(true);
            return;
        }

        if(err == StatusCodes::GateFailed){
            showTip(false, tr("GateServer服务连接失败"));
            enableBtn(true);
            return;
        }

        if(err == StatusCodes::StatusFailed){
            showTip(false, tr("StatusServer服务连接失败"));
            enableBtn(true);
            return;
        }

        if(err != StatusCodes::Success){
            showTip(false, tr("不可预知的错误"));
            enableBtn(true);
            return;
        }

        auto user = jsonObj["user"].toString();

        //发送信号通知tcpMgr发送长链接
        ServerInfo si;
        //si.uid = jsonObj["uid"].toInt();
        si.host = jsonObj["host"].toString();
        si.port = jsonObj["port"].toString();
        //si.token = jsonObj["token"].toString();

        uid = jsonObj["uid"].toInt();
        token = jsonObj["token"].toString();

        qDebug() << "LoginDialog::initHandlers() REQ_USER_LOGIN: user" << user << ", uid" << uid
                 << ", host" << si.host << ", port" << si.port << ", token" << token;
        showTip(true, tr("登录成功，正在连接服务器..."));
        emit sig_tcp_connect(si);
    });
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, StatusCodes statusCode)
{
    qDebug() << "LoginDialog::slot_login_mod_finish()";
    if(statusCode == StatusCodes::GateFailed){
        showTip(false, tr("GateServer服务连接失败"));
        enableBtn(true);
        return;
    }

    if(statusCode == StatusCodes::StatusFailed){
        showTip(false, tr("StatusServer服务连接失败"));
        enableBtn(true);
        return;
    }

    if(statusCode != StatusCodes::Success){
        showTip(false, tr("不可预知的错误"));
        enableBtn(true);
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(false, tr("json解析错误"));
        enableBtn(true);
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(false, tr("json解析错误"));
        enableBtn(true);
        return;
    }


    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());

    return;
}

void LoginDialog::slot_tcp_conn_fin(bool b_success){
    qDebug() << "LoginDialog::slot_tcp_conn_fin()" << b_success;
    if(b_success){
        showTip(true, tr("已连接到聊天服务器，发送登录请求..."));
        QJsonObject jsonObj;
        jsonObj["uid"] = uid;
        jsonObj["token"] = token;

        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

        //发送tcp请求给ChatServer
        //tcp内容是登录到聊天服务器所需的uid和token
        emit TcpMgr::GetInstance()->sig_tcp_send_data(ReqId::REQ_CHAT_LOGIN, jsonData);

    }else{
        showTip(false, tr("ChatServer服务连接失败"));
        enableBtn(true);
    }
}

void LoginDialog::slot_login_failed(int statusCode){
    qDebug() << "LoginDialog::slot_login_failed() err" << statusCode;

    if(statusCode == StatusCodes::LoginHandlerFailed){
        showTip(false, tr("无法匹配到处理登录请求的函数"));
        enableBtn(true);
        return;
    }

    if(statusCode == StatusCodes::TokenInvalid){
        showTip(false, tr("Token失效，登录失败"));
        enableBtn(true);
        return;
    }

    if(statusCode == StatusCodes::UidInvalid){
        showTip(false, tr("Uid失效，登录失败"));
        enableBtn(true);
        return;
    }

    if(statusCode != StatusCodes::Success){
        showTip(false, tr("登录请求失败"));
        enableBtn(true);
        return;
    }

    enableBtn(true);
}

void LoginDialog::initHead()
{
    // 加载图片
    QPixmap originalPixmap(":/res/login_head.jpg");
    // 设置图片自动缩放
    qDebug()<< originalPixmap.size() << ui->head_label->size();
    originalPixmap = originalPixmap.scaled(ui->head_label->size(),
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建一个和原始图片相同大小的QPixmap，用于绘制圆角图片
    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent); // 用透明色填充

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing); // 设置抗锯齿，使圆角更平滑
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 使用QPainterPath设置圆角
    QPainterPath path;
    path.addRoundedRect(0, 0, originalPixmap.width(), originalPixmap.height(), 10, 10); // 最后两个参数分别是x和y方向的圆角半径
    painter.setClipPath(path);

    // 将原始图片绘制到roundedPixmap上
    painter.drawPixmap(0, 0, originalPixmap);

    // 设置绘制好的圆角图片到QLabel上
    ui->head_label->setPixmap(roundedPixmap);

}

void LoginDialog::slot_forget_passwd() {
    qDebug() << "LoginDialog::slot_forget_passwd()";
    emit sig_switch_reset();
}

//点击登录按钮
//USER接收用户名和邮箱
void LoginDialog::on_login_button_clicked() {
    qDebug()<<"login btn clicked";
    if(checkUserValid() == false){
        return;
    }

    if(checkPswdValid() == false){
        return ;
    }

    //通过，禁用注册、登录按钮，以及忘记密码label
    enableBtn(false);
    auto user_or_email = ui->username_edit->text();
    auto pswd = ui->passwd_edit->text();
    //发送http请求登录
    QJsonObject json_obj;
    json_obj["user"] = user_or_email;
    json_obj["pswd"] = xorString(pswd);
    HttpMgr::GetInstance()->postHttpReq(QUrl(GateServer_url_perfix + "/user_login"),
                                        json_obj, ReqId::REQ_USER_LOGIN, Modules::MOD_LOGIN);
    showTip(true, tr("正在登录..."));

}

bool LoginDialog::enableBtn(bool enabled) {
    ui->login_button->setEnabled(enabled);
    ui->register_button->setEnabled(enabled);
    if(enabled){
        connect(ui->forget_passwd_label, &ClickLabel::clicked, this, &LoginDialog::slot_forget_passwd);
    }
    else{
        disconnect(ui->forget_passwd_label, &ClickLabel::clicked, this, &LoginDialog::slot_forget_passwd);
    }
    return true;
}

//用户名判断，允许用户名和邮箱输入
//这里放宽了限制，只判断非空
bool LoginDialog::checkUserValid() {
    auto user_or_email = ui->username_edit->text();
    if(user_or_email.isEmpty()){
        qDebug() << "user_or_email" << user_or_email << "is empty";
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("用户名不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool LoginDialog::checkPswdValid() {
    auto pswd = ui->passwd_edit->text();

    if(pswd.isEmpty()){
        AddTipErr(TipErr::TIP_PSWD_ERR, tr("密码不能为空"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{1,15}$ 密码长度至少4，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{1,15}$");
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

void LoginDialog::showTip(bool stat, QString str){
    if(stat){
        ui->msg_output_label->setProperty("state", "normal");
    }
    else{
        ui->msg_output_label->setProperty("state", "error");
    }
    ui->msg_output_label->setText(str);
    repolish(ui->msg_output_label);
}

void LoginDialog::AddTipErr(TipErr te, QString tips){
    tipErrs[te] = tips;
    showTip(false, tips);
}

void LoginDialog::DelTipErr(TipErr te){
    tipErrs.remove(te);

    if(tipErrs.empty()){
        ui->msg_output_label->clear();
        return;
    }

    showTip(false, tipErrs.first());
}
