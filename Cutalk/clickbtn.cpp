#include "clickbtn.h"
#include "defs.h"
#include <QVariant>
#include <QKeyEvent>

ClickBtn::ClickBtn(QWidget *parent): QPushButton (parent) {
    //设置光标为手状
    setCursor(Qt::PointingHandCursor);
    //阻止键盘将其设置为焦点，提升性能
    setFocusPolicy(Qt::NoFocus);
}

ClickBtn::~ClickBtn(){

}


void ClickBtn::SetState(QString normal, QString hover, QString press)
{
    _hover = hover;
    _normal = normal;
    _press = press;
    setProperty("state", normal);
    repolish(this);
    update();
}

void ClickBtn::enterEvent(QEvent *event)
{
    setProperty("state", _hover);
    repolish(this);
    update();
    QPushButton::enterEvent(event);
}

void ClickBtn::leaveEvent(QEvent *event)
{
    setProperty("state", _normal);
    repolish(this);
    update();
    QPushButton::leaveEvent(event);
}

void ClickBtn::mousePressEvent(QMouseEvent *event)
{
    setProperty("state", _press);
    repolish(this);
    update();
    QPushButton::mousePressEvent(event);
}

void ClickBtn::mouseReleaseEvent(QMouseEvent *event)
{
    setProperty("state", _hover);
    repolish(this);
    update();
    QPushButton::mouseReleaseEvent(event);
}
