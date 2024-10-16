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
}

MainWindow::~MainWindow()
{
    delete ui;
}
