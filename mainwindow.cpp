#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _login_dialog = new LoginDialog();
    setCentralWidget(_login_dialog);
    _login_dialog->show();

    //connect _login_dialog.switchRegister() to this->slotSwitchRegister()
    //connect(sender, &SenderClass::signalName, receiver, &ReceiverClass::slotName);
    connect(_login_dialog, &LoginDialog::switchRegister, this, &MainWindow::slotSwitchRegister);
    _register_dialog = new RegisterDialog();

}

MainWindow::~MainWindow()
{
    delete ui;

    if(_login_dialog){
        delete _login_dialog;
        _login_dialog = nullptr;
    }

    if(_register_dialog){
        delete _register_dialog;
        _register_dialog = nullptr;
    }
}

void MainWindow::slotSwitchRegister()
{
    setCentralWidget(_register_dialog);
    _login_dialog->hide();
    _register_dialog->show();
}
