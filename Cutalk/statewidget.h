#ifndef STATEWIDGET_H
#define STATEWIDGET_H
#include <QWidget>
#include "defs.h"
#include <QLabel>

class StateWidget: public QWidget
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget *parent = nullptr);

    void init(QString normal="", QString hover="", QString press="",
              QString select="", QString select_hover="", QString select_press="");

    LabelClickState GetCurState();
    void SetCurState(LabelClickState lcs);
    void ClearState();

    void SetSelected(bool bselected);
    void AddRedPoint();
    void ShowRedPoint(bool show=true);

protected:
    void paintEvent(QPaintEvent* event) override;

    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
    virtual void enterEvent(QEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

private:

    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    LabelClickState _curstate;
    QLabel * _red_point;

signals:
    void clicked(void);

signals:

public slots:
};

#endif // STATEWIDGET_H