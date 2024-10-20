#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _login_dialog = new LoginDialog(this);
    _register_dialog = new RegisterDialog(this);

    setCentralWidget(_login_dialog);
    //_login_dialog->show();

    //connect _login_dialog.switchRegister() to this->slot_switch_register()
    //connect(sender, &SenderClass::signalName, receiver, &ReceiverClass::slotName);
    connect(_login_dialog, &LoginDialog::switchRegister, this, &MainWindow::slot_switch_register);

    _login_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    _register_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

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
    setCentralWidget(_register_dialog);
    //_login_dialog->hide();
    //_register_dialog->show();
}
