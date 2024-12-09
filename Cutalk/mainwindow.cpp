#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _login_dialog = new LoginDialog(this);
    //_register_dialog = new RegisterDialog(this);

    //setCentralWidget(_login_dialog);
    //_login_dialog->show();

    //connect _login_dialog.sig_switch_register() to this->slot_switch_register()
    //connect(sender, &SenderClass::signalName, receiver, &ReceiverClass::slotName);
    connect(_login_dialog, &LoginDialog::sig_switch_register, this, &MainWindow::slot_switch_register);

    _login_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dialog);
    //_register_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    //_register_dialog->hide();

    connect(_login_dialog, &LoginDialog::sig_switch_reset, this, &MainWindow::slot_switch_reset);

}

MainWindow::~MainWindow()
{
    delete ui;

    /*
    if(_login_dialog){
        delete _login_dialog;
        _login_dialog = nullptr;
    }

    if(_register_dialog){
        delete _register_dialog;
        _register_dialog = nullptr;
    }*/
}

void MainWindow::slot_switch_register()
{
    _register_dialog = new RegisterDialog(this);
    //_login_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    //_register_dialog->hide();
    _register_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    //连接注册界面Page2返回登录事件和登录界面
    connect(_register_dialog, &RegisterDialog::sig_switch_login, this, &MainWindow::slot_switch_login_from_register_page2);

    setCentralWidget(_register_dialog);
    _login_dialog->hide();
    //_register_dialog->show();
}

//从注册界面page2返回登录界面
void MainWindow::slot_switch_login_from_register_page2()
{
    qDebug() << "MainWindow::slot_switch_login_from_register_page2()";
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dialog = new LoginDialog(this);
    _login_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dialog);

    _register_dialog->hide();
    _login_dialog->show();

    //登录界面相关的信号槽
    //连接登录界面注册信号
    connect(_login_dialog, &LoginDialog::sig_switch_register,
            this, &MainWindow::slot_switch_register);
    //连接登录界面忘记密码信号
    connect(_login_dialog, &LoginDialog::sig_switch_reset,
            this, &MainWindow::slot_switch_reset);
}

//从主界面点击忘记密码跳转到重置密码界面
void MainWindow::slot_switch_reset(){
    qDebug() << "MainWindow::slot_switch_reset()";
    //new一个reset界面
    _reset_dialog = new ResetDialog(this);
    _reset_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_reset_dialog);

    _login_dialog->hide();
    _reset_dialog->show();

    //注册密码重置界面的信号槽
    //注册返回登录信号和槽函数
    connect(_reset_dialog, &ResetDialog::sig_switch_login,
            this, &MainWindow::slot_switch_login_from_forget);

}

//从忘记密码重置界面返回登录界面
void MainWindow::slot_switch_login_from_forget()
{
    qDebug() << "MainWindow::slot_switch_login_from_forget()";
    _login_dialog = new LoginDialog(this);
    _login_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dialog);

    _reset_dialog->hide();
    _login_dialog->show();

    //连接登录界面忘记密码信号
    connect(_login_dialog, &LoginDialog::sig_switch_reset, this, &MainWindow::slot_switch_reset);
    //连接登录界面注册信号
    connect(_login_dialog, &LoginDialog::sig_switch_register, this, &MainWindow::slot_switch_register);
}
