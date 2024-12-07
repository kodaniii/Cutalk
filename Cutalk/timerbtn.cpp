#include "timerbtn.h"
#include <QMouseEvent>
#include <QDebug>
#include <QRegularExpression>

TimerBtn::TimerBtn(QWidget *parent): QPushButton(parent), counter(5) {
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, [this](){
        counter--;
        if(counter <= 0){
            timer->stop();
            counter = 5;
            this->setText("获取");
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(counter));
    });
}

TimerBtn::~TimerBtn() {
    timer->stop();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        // 在这里处理鼠标左键释放事件
        qDebug() << "TimerBtn::mouseReleaseEvent() leftButton released.";
        this->setEnabled(false);
        this->setText(QString::number(counter));
        timer->start(1000);
        emit clicked();
    }
    // 调用基类的mouseReleaseEvent以确保正常的事件处理（如点击效果）
    QPushButton::mouseReleaseEvent(e);
}
