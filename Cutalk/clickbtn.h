#ifndef CLICKBTN_H
#define CLICKBTN_H

#include <QPushButton>

class ClickBtn: public QPushButton
{
    Q_OBJECT
public:
    ClickBtn(QWidget *parent = nullptr);
    ~ClickBtn();
    void init(QString nomal, QString hover, QString press);
protected:
    //鼠标进入
    virtual void enterEvent(QEvent *event) override;
    //鼠标离开
    virtual void leaveEvent(QEvent *event) override;
    //鼠标按下
    virtual void mousePressEvent(QMouseEvent *event) override;
    //鼠标释放
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
private:
    QString _normal;
    QString _hover;
    QString _press;
};

#endif // CLICKEDBTN_H
