#include "clickoncelabel.h"
#include <QMouseEvent>

ClickOnceLabel::ClickOnceLabel(QWidget *parent)
 : QLabel(parent) {
    setCursor(Qt::PointingHandCursor);
}

void ClickOnceLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked(this->text());
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(ev);
}
