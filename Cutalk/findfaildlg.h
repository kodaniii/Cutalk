#ifndef FINDFAILDLG_H
#define FINDFAILDLG_H

#include <QDialog>

namespace Ui {
class FindFailDlg;
}

class FindFailDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FindFailDlg(QWidget *parent = nullptr);
    ~FindFailDlg();

protected:
    /*窗口拖动逻辑*/
    void mousePressEvent(QMouseEvent *event) override; // 鼠标按下事件
    void mouseMoveEvent(QMouseEvent *event) override;  // 鼠标移动事件
    void mouseReleaseEvent(QMouseEvent *event) override; // 鼠标释放事件

private slots:
    void on_fail_sure_btn_clicked();

private:
    Ui::FindFailDlg *ui;
    QPoint m_dragPosition; // 用于记录鼠标拖动时的位置
    bool m_isDragging;     // 是否正在拖动窗口
};

#endif // FINDFAILDLG_H
