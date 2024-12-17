/******************************************************************************
 *
 * @file       mainwindow.h
 * @brief      MainWindow
 *
 * @author     jiang
 * @date       2024/10/16
 * @history
 *****************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "registerdialog.h"
#include "resetdialog.h"
#include "chatdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    LoginDialog *_login_dialog;
    RegisterDialog *_register_dialog;
    ResetDialog *_reset_dialog;
    ChatDialog *_chat_dialog;

public slots:
    void slot_switch_register();
    void slot_switch_login_from_register_page2();

    void slot_switch_reset();
    void slot_switch_login_from_forget();

    void slot_switch_chatdlg();
};
#endif // MAINWINDOW_H
