#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tcpmgr.h"
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _chat_dialog(nullptr)
{
    ui->setupUi(this);

    //setWindowFlags(Qt::FramelessWindowHint);

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

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_switch_chatdlg, this, &MainWindow::slot_switch_chatdlg);
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


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        m_isDragging = true;
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    /*修复聊天窗口侧边栏移动过小导致窗口跳动的BUG
    有效果，但还不是完全有效果，所以这里代码也没删，
    在statewidget中将mousePressEvent和mouseMoveEvent做了忽略
    */
    if (event->buttons() & Qt::LeftButton && m_isDragging)
    {
        // 计算新的位置
        QPoint newPos = event->globalPos() - m_dragPosition;

        // 计算当前位置与新位置之间的距离
        int distance = (newPos - this->pos()).manhattanLength();

        // 设置一个阈值，只有当移动距离大于阈值时才移动窗口
        const int threshold = 5;
        if (distance > threshold)
        {
            move(newPos);
            //qDebug() << "move to" << newPos;
        }
    }
    /*
    if (event->buttons() & Qt::LeftButton && m_isDragging)
    {
        move(event->globalPos() - m_dragPosition);
    }*/
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDragging = false;
    }
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

void MainWindow::slot_switch_chatdlg()
{
    qDebug() << "MainWindow::slot_switch_chatdlg()";

    _chat_dialog = new ChatDialog();
    _chat_dialog->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_chat_dialog);
    _chat_dialog->show();
    _login_dialog->hide();

    //QScreen *screen = QGuiApplication::primaryScreen();
    //qreal dpiScale = screen->logicalDotsPerInchX() / 96.0;
    //this->setMinimumSize(QSize(700 * dpiScale, 600 * dpiScale));
    //qDebug() << "setMinimumSize" << 700 * dpiScale << 600 * dpiScale;

    this->setMinimumSize(QSize(1050, 900));
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

