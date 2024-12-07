#ifndef TIMERBTN_H
#define TIMERBTN_H
#include <QPushButton>
#include <QTimer>

class TimerBtn: public QPushButton
{
public:
    TimerBtn(QWidget *parent = nullptr);
    ~TimerBtn();

    //重写QAbstractButton::mouseReleaseEvent
    //鼠标抬起事件
    virtual void mouseReleaseEvent(QMouseEvent *e) override;

private:
    QTimer *timer;
    int counter;
};

#endif // TIMERBTN_H
