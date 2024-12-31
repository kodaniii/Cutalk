#include "adduseritem.h"
#include "ui_adduseritem.h"
#include <QLabel>
#include <QTimer>

AddUserItem::AddUserItem(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::AddUserItem)
    , _movie(nullptr)
{
    ui->setupUi(this);
    SetItemType(ListItemType::ADD_USER_TIP_ITEM);


    QPixmap pixmap(":/res/right_tip.png");
    ui->right_tip->setPixmap(pixmap);
}

AddUserItem::~AddUserItem()
{
    delete ui;
}

void AddUserItem::loading_user_search_rsp(bool pending)
{
    qDebug() << "AddUserItem::slot_loading_user_search_rsp() pending" << pending;
    if(pending){
        /*loading gif显示，替换QLabel right_tip*/
        _movie = new QMovie(":/res/loading.gif");

        ui->right_tip->clear();
        ui->right_tip->setMovie(_movie);
        ui->right_tip->setFixedSize(40, 40);
        ui->right_tip->setAlignment(Qt::AlignCenter);
        _movie->setScaledSize(QSize(40, 40));
        _movie->start();
    }
    else{
        set_search_loading_to_right();
    }
}

void AddUserItem::set_search_loading_to_right()
{
    if(_movie == nullptr){
        return;
    }

    _movie->stop();
    //清除QMovie对象
    ui->right_tip->clear();

    QPixmap pixmap(":/res/right_tip.png");
    ui->right_tip->setPixmap(pixmap);
    ui->right_tip->setFixedSize(32, 32);

    _movie = nullptr;
}
