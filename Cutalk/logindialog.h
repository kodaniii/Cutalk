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

signals:
    //切换到注册界面
    void sig_switch_register();
    //切换到重置密码界面，通过主页忘记密码跳转
    void sig_switch_reset();

public slots:
    void slot_forget_passwd();
};

#endif // LOGINDIALOG_H
