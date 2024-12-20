#ifndef FINDSUCCESSDLG_H
#define FINDSUCCESSDLG_H

#include <QDialog>
#include "userdata.h"
#include <memory>

namespace Ui {
class FindSuccessDlg;
}

class FindSuccessDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FindSuccessDlg(QWidget *parent = nullptr);
    ~FindSuccessDlg();

    void SetSearchInfo(std::shared_ptr<SearchInfo> si);

protected:
    /*窗口拖动逻辑*/
    void mousePressEvent(QMouseEvent *event) override; // 鼠标按下事件
    void mouseMoveEvent(QMouseEvent *event) override;  // 鼠标移动事件
    void mouseReleaseEvent(QMouseEvent *event) override; // 鼠标释放事件

private slots:
    void on_add_friend_btn_clicked();

private:
    Ui::FindSuccessDlg *ui;

    QPoint m_dragPosition; // 用于记录鼠标拖动时的位置
    bool m_isDragging;     // 是否正在拖动窗口

    QWidget *_parent;
    std::shared_ptr<SearchInfo> _si;


};

#endif // FINDSUCCESSDLG_H
