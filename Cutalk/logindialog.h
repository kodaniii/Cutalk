/******************************************************************************
 *
 * @file       logindialog.h
 * @brief      login
 *
 * @author     jiang
 * @date       2024/10/16
 * @history
 *****************************************************************************/
#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "defs.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    Ui::LoginDialog *ui;

    void initHead();
    bool checkUserValid();
    bool checkPswdValid();
    bool enableBtn(bool);

    void showTip(bool, QString);
    void AddTipErr(TipErr te, QString str);
    void DelTipErr(TipErr te);
    QMap<TipErr, QString> tipErrs;

    void initHandlers();
    QMap<HttpReqId, std::function<void(const QJsonObject&)>> _handlers;

signals:
    //切换到注册界面
    void sig_switch_register();
    //切换到重置密码界面，通过主页忘记密码跳转
    void sig_switch_reset();

    //发起http长连接
    void sig_connect_tcp(ServerInfo);

public slots:
    void slot_forget_passwd();

private slots:
    void on_login_button_clicked();

    void slot_login_mod_finish(HttpReqId, QString, StatusCodes);
};

#endif // LOGINDIALOG_H
