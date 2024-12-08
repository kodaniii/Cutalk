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

    //connect _login_dialog.switchRegister() to this->slot_switch_register()
    //connect(sender, &SenderClass::signalName, receiver, &ReceiverClass::slotName);
    connect(_login_dialog, &LoginDialog::switchRegister, this, &MainWindow::slot_switch_register);

    _login_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dialog);
    //_register_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    //_register_dialog->hide();
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
    connect(_register_dialog, &RegisterDialog::sig_switch_login, this, &MainWindow::slot_switch_login);

    setCentralWidget(_register_dialog);
    _login_dialog->hide();
    //_register_dialog->show();
}

//从注册界面page2返回登录界面
void MainWindow::slot_switch_login()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dialog = new LoginDialog(this);
    _login_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dialog);

    //连接登录界面注册信号
    connect(_login_dialog, &LoginDialog::switchRegister, this, &MainWindow::slot_switch_register);

    _register_dialog->hide();
    _login_dialog->show();
}
