#include "findfaildlg.h"
#include "ui_findfaildlg.h"
#include <QMouseEvent>

FindFailDlg::FindFailDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindFailDlg)
{
    ui->setupUi(this);
    setWindowTitle("添加联系人");
    //隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("FindFailDlg");

    ui->fail_sure_btn->init("normal","hover","press");

    this->setModal(true);
}

FindFailDlg::~FindFailDlg()
{
    delete ui;
}


void FindFailDlg::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        m_isDragging = true;
    }
}

void FindFailDlg::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && m_isDragging)
    {
        move(event->globalPos() - m_dragPosition);
    }
}

void FindFailDlg::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDragging = false;
    }
}

void FindFailDlg::on_fail_sure_btn_clicked()
{
    this->hide();
}
