#include "searchlist.h"
#include <QScrollBar>
#include "adduseritem.h"
//#include "invaliditem.h"
//#include "findsuccessdlg.h"
#include "tcpmgr.h"
#include "customizeedit.h"
//#include "findfaildlg.h"
//#include "loadingdlg.h"
#include "userdata.h"
#include <QJsonDocument>
#include "findsuccessdlg.h"

SearchList::SearchList(QWidget *parent)
    : QListWidget(parent)
    , _send_pending(false)
    , _find_dlg(nullptr)
    , _search_edit(nullptr)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    //每个条目被点击都会触发solt_item_clicked
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);
    //测试：模拟添加条目
    addTipItem();

    //用户搜索
    //void sig_user_search(std::shared_ptr<SearchInfo>);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search, this, &SearchList::slot_user_search);
}

void SearchList::CloseFindDlg()
{
    if(_find_dlg){
        _find_dlg->hide();
        _find_dlg = nullptr;
    }
}

void SearchList::SetSearchEdit(QWidget* edit) {
    _search_edit = edit;
}

void SearchList::waitPending(bool pending)
{
    // if(pending){
    //     _loadingDialog = new LoadingDlg(this);
    //     _loadingDialog->setModal(true);
    //     _loadingDialog->show();
    //     _send_pending = pending;
    // }else{
    //     _loadingDialog->hide();
    //     _loadingDialog->deleteLater();
    //     _send_pending = pending;
    // }
}


void SearchList::addTipItem()
{
    auto *invalid_item = new QWidget();
    QListWidgetItem *item_tmp = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item_tmp->setSizeHint(QSize(280, 1));
    this->addItem(item_tmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_tmp, invalid_item);
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);


    auto *add_user_item = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, add_user_item);
}

void SearchList::slot_item_clicked(QListWidgetItem *item)
{
    qDebug() << "SearchList::slot_item_clicked()";
    QWidget *widget = this->itemWidget(item); // 获取自定义widget对象
    if(!widget){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    // 对自定义widget进行操作， 将item转化为子类ListItemBase，为了调用GetItemType()
    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if(itemType == ListItemType::INVALID_ITEM){
        qDebug()<< "slot invalid item clicked ";
        return;
    }

    if(itemType == ListItemType::ADD_USER_TIP_ITEM){
        qDebug() << "itemType == ListItemType::ADD_USER_TIP_ITEM";
        //todo ...
        _find_dlg = std::make_shared<FindSuccessDlg>(this);
        /* 搜索到添加好友
         * 用户信息用于测试
        */
        auto si = std::make_shared<SearchInfo>(0, "可爱大苹果", "大可爱苹果", "别加我，我只是个传说", 0, "none");

        //存储
        std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg)->SetSearchInfo(si);
        _find_dlg->show();
        return;

        /*
        if (_send_pending) {
            return;
        }

        if (!_search_edit) {
            return;
        }
        waitPending(true);
        auto search_edit = dynamic_cast<CustomizeEdit*>(_search_edit);
        auto uid_str = search_edit->text();
        //此处发送请求给server
        QJsonObject jsonObj;
        jsonObj["uid"] = uid_str;

        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        //发送tcp请求给ChatServer
        emit TcpMgr::GetInstance()->sig_tcp_send_data(ReqId::REQ_SEARCH_USER, jsonData);
        return;*/
    }

    //清除弹出框
    CloseFindDlg();
}

void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si)
{
}
