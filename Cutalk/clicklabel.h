#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include "defs.h"
#include <QKeyEvent>

class ClickLabel : public QLabel
{
    Q_OBJECT
public:
    ClickLabel(QWidget *parent = nullptr);

    //鼠标点击事件
    virtual void mousePressEvent(QMouseEvent *ev) override;
    //鼠标取消点击
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
    //鼠标悬停进入事件
    virtual void enterEvent(QEvent* event) override;
    //鼠标悬停离开事件
    virtual void leaveEvent(QEvent* event) override;

    //state一共有6种
    //分为两类，Unselected未选中和Selected选中（体现为睁眼、不睁眼的小logo）
    //每类有三种状态，普通状态、指针悬浮在上面的状态hover、点击状态press
    void init(QString _unselected="", QString _unselected_hover="", QString _unselected_press="",
              QString _selected="", QString _selected_hover="", QString _selected_press="");

    LabelClickState GetCurState();
    bool SetCurState(LabelClickState state);
    void ResetNormalState();


private:
    //闭眼状态
    QString unselected;
    QString unselected_hover;
    QString unselected_press;

    //睁眼状态
    QString selected;
    QString selected_hover;
    QString selected_press;

    LabelClickState cur_state;

signals:
    void clicked(QString, LabelClickState);
};

#endif // CLICKLABEL_H
