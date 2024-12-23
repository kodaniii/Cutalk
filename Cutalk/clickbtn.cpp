#include "clickbtn.h"
#include "defs.h"
#include <QVariant>
#include <QKeyEvent>

ClickBtn::ClickBtn(QWidget *parent): QPushButton (parent) {
    //设置光标为手状
    setCursor(Qt::PointingHandCursor);
    //阻止键盘将其设置为焦点
    //ApplyFriend中申请好友，在edit中添加信息后，处理用户敲入回车的操作
    //这里阻止敲入回车是为了把回车截获取消掉，防止回车后ApplyFriend界面消失
    setFocusPolicy(Qt::NoFocus);
}

ClickBtn::~ClickBtn(){

}


void ClickBtn::init(QString normal, QString hover, QString press)
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
