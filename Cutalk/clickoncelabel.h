#ifndef CLICKONCELABEL_H
#define CLICKONCELABEL_H

#include <QLabel>

class ClickOnceLabel : public QLabel
{
    Q_OBJECT
public:
    ClickOnceLabel(QWidget *parent = nullptr);
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;

signals:
    void clicked(QString);
};

#endif // CLICKONCELABEL_H
