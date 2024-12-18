#ifndef LISTITEMBASE_H
#define LISTITEMBASE_H
#include <QWidget>
#include "defs.h"

class ListItemBase: public QWidget
{
    Q_OBJECT
public:
    explicit ListItemBase(QWidget *parent = nullptr);
    void SetItemType(ListItemType itemType);

    ListItemType GetItemType();

protected:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    ListItemType _itemType;

public slots:

signals:


};

#endif // LISTITEMBASE_H
