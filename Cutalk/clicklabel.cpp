#include "clicklabel.h"
#include <QMouseEvent>

ClickLabel::ClickLabel(QWidget *parent): QLabel(parent), cur_state(LabelClickState::Unselected) {
    //鼠标移动到该label时，变为手状
    //所有提升为ClickLabel类的label
    this->setCursor(Qt::PointingHandCursor);
}

void ClickLabel::init(QString _unselected, QString _unselected_hover, QString _unselected_press,
                      QString _selected, QString _selected_hover, QString _selected_press) {
    unselected = _unselected;
    unselected_hover = _unselected_hover;
    unselected_press = _unselected_press;

    selected = _selected;
    selected_hover = _selected_hover;
    selected_press = _selected_press;

    setProperty("state", _unselected);
    repolish(this);
}

//将两种状态设置为悬停，并保存非悬停的状态
//点击时鼠标一定悬停
void ClickLabel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if(cur_state == LabelClickState::Unselected){
            qDebug()<<"ClickLabel::mousePressEvent(), change to" << selected_press;
            cur_state = LabelClickState::Selected;
            setProperty("state", selected_press);
            repolish(this);
            update();
        }else{
            qDebug()<<"ClickLabel::mousePressEvent(), change to" << unselected_press;
            cur_state = LabelClickState::Unselected;
            setProperty("state", unselected_press);
            repolish(this);
            update();
        }
        //emit clicked(this->text(), cur_state);
        //忽略事件，防止全局拖动功能重叠导致页面乱跳
        event->ignore();
        return;
    }

    QLabel::mousePressEvent(event);
}

//鼠标点击释放
//取消press，返回对应的hover或normal
void ClickLabel::mouseReleaseEvent(QMouseEvent *event) {
    qDebug() << "ClickLabel::mouseReleaseEvent()";
    //enterEvent(event);
    if (event->button() == Qt::LeftButton) {
        if(cur_state == LabelClickState::Unselected){
            setProperty("state", unselected_hover);
            repolish(this);
            update();

        }else{
            setProperty("state", selected_hover);
            repolish(this);
            update();
        }
        emit clicked(this->text(), cur_state);
        //忽略事件，防止全局拖动功能重叠导致页面乱跳
        event->ignore();
        return;
    }
    QLabel::mouseReleaseEvent(event);
}

//将两种状态设置为悬停
void ClickLabel::enterEvent(QEvent* event) {
    if(cur_state == LabelClickState::Unselected){
        //qDebug() << "ClickLabel::enterEvent(), change to" << unselected_hover;
        setProperty("state", unselected_hover);
        repolish(this);
        update();
    }else{
        //qDebug() << "ClickLabel::enterEvent(), change to" << selected_hover;
        setProperty("state", selected_hover);
        repolish(this);
        update();
    }

    QLabel::enterEvent(event);
}

//两种状态设置为非悬停
void ClickLabel::leaveEvent(QEvent* event) {
    if(cur_state == LabelClickState::Unselected){
        //qDebug() << "ClickedLabel::leaveEvent(), change to" << unselected;
        setProperty("state", unselected);
        repolish(this);
        update();
    }else{
        //qDebug() << "ClickedLabel::leaveEvent(), change to" << selected;
        setProperty("state", selected);
        repolish(this);
        update();
    }

    QLabel::leaveEvent(event);
}

LabelClickState ClickLabel::GetCurState() {
    return this->cur_state;
}

bool ClickLabel::SetCurState(LabelClickState state)
{
    cur_state = state;
    if (cur_state == LabelClickState::Unselected) {
        setProperty("state", unselected);
        repolish(this);
    }
    else if (cur_state == LabelClickState::Selected) {
        setProperty("state", selected);
        repolish(this);
    }

    return true;
}

void ClickLabel::ResetNormalState()
{
    cur_state = LabelClickState::Unselected;
    setProperty("state", unselected);
    repolish(this);
}
