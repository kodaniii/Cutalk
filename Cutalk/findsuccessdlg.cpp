#include "findsuccessdlg.h"
#include "ui_findsuccessdlg.h"
#include <QDir>
#include <QMouseEvent>

FindSuccessDlg::FindSuccessDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindSuccessDlg)
{
    ui->setupUi(this);
    // 设置对话框标题
    setWindowTitle("添加联系人");
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("FindSuccessDlg");
    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    //临时用来测试的图片路径
    QString pix_path = QDir::toNativeSeparators(app_path +
                                                QDir::separator() + "res" +
                                                QDir::separator() + "static" +
                                                QDir::separator() + "head_7.jpg");
    QPixmap head_pix(pix_path);
    head_pix = head_pix.scaled(ui->head_lb->size(),
                               Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->head_lb->setPixmap(head_pix);
    ui->add_friend_btn->init("normal","hover","press");
    this->setModal(true);
}

FindSuccessDlg::~FindSuccessDlg()
{
    delete ui;
}


void FindSuccessDlg::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        m_isDragging = true;
    }
}

void FindSuccessDlg::mouseMoveEvent(QMouseEvent *event)
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

void FindSuccessDlg::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDragging = false;
    }
}


void FindSuccessDlg::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    ui->name_lb->setText(si->_name);
    _si = si;
}

void FindSuccessDlg::on_add_friend_btn_clicked()
{
    //TODO ...
}
