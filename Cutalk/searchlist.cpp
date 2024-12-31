#include "searchlist.h"
#include <QScrollBar>
//#include "invaliditem.h"
//#include "findsuccessdlg.h"
#include "tcpmgr.h"
#include "customizeedit.h"
//#include "findfaildlg.h"
//#include "loadingdlg.h"
#include "userdata.h"
#include <QJsonDocument>
#include "findsuccessdlg.h"
#include "findfaildlg.h"
#include <QLabel>
#include <QMovie>
#include <QTimer>

SearchList::SearchList(QWidget *parent)
    : QListWidget(parent)
    , _send_pending(false)
    , _find_dlg(nullptr)
    , _search_edit(nullptr)
    , _add_user_item(nullptr)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    //每个条目被点击都会触发solt_item_clicked
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);

    //初始化search_list的搜索项：查找uid/用户名项
    initTipItem();

    //用户搜索
    //void sig_user_search(std::shared_ptr<SearchInfo>);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search,
            this, &SearchList::slot_user_search);
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

    _send_pending = pending;
    _add_user_item->loading_user_search_rsp(_send_pending);
}


void SearchList::initTipItem()
{
    auto *invalid_item = new QWidget();
    QListWidgetItem *item_tmp = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    item_tmp->setSizeHint(QSize(280, 1));
    this->addItem(item_tmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_tmp, invalid_item);
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);


    _add_user_item = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(_add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, _add_user_item);
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

        //todo
        if (_send_pending) {
            qDebug() << "_send_pending true, return...";
            return;
        }

        if (!_search_edit) {
            return;
        }
        //waitPending(true);

        //等待TCP传回包，如果没有对应用户结果的回包，就会一直转圈圈等待
        waitPending(true);

        /*
        //test 2.024秒后收到包，这里实际应该是在tcpmgr handler里面处理
        QTimer::singleShot(2024, this, [this](){
            qDebug() << "[test] SearchList adduseritem loading_user_search_rsp";
            waitPending(false);
        });*/

        auto search_edit = dynamic_cast<CustomizeEdit*>(_search_edit);
        auto uid_or_name_str = search_edit->text();
        qDebug() << "get search_edit->text()" << uid_or_name_str;

        //此处发送请求给server
        QJsonObject jsonObj;
        jsonObj["uid_or_name"] = uid_or_name_str;

        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);   //压缩

        //发送tcp请求给ChatServer
        //搜索某个用户，tcp内容是这个用户的uid
        emit TcpMgr::GetInstance()->sig_tcp_send_data(ReqId::REQ_SEARCH_USER, jsonData);
        return;

        /* 测试：直接生成一个模拟find_dlg
         * 后期需要将这里搬到searchlist slot_user_search中处理，而不是RSP回包处理函数
        _find_dlg = std::make_shared<FindSuccessDlg>(this);

        //搜索到添加好友
        //用户信息用于测试
        auto si = std::make_shared<SearchInfo>(0, "可爱大苹果", "大可爱苹果", "别加我，我只是个传说", 0, "none");

        //存储
        std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg)->SetSearchInfo(si);
        _find_dlg->show();
        return;
        */

    }

    //清除弹出框
    //CloseFindDlg();
}

void SearchList::slot_user_search(bool stat, std::shared_ptr<SearchInfo> si)
{
    qDebug() << "SearchList::slot_user_search() stat" << stat
             << "SearchInfo nullptr?" << (si == nullptr);
    //这里不做动画了（后来还是做了）
    //waitPending(false);

    waitPending(false);

    //搜索成功stat为true，即使没有查找到对应用户也为true
    if(stat == false){
        //可能是json解析等错误
        return;
    }

    if(si == nullptr){
        //没有对应用户
        _find_dlg = std::make_shared<FindFailDlg>(this);
    }
    else{
        //找到对应用户，分为两种情况
        //1、该用户已经是自己的好友
        //2、该用户不是自己的好友

        //TODO 判断是不是好友

        _find_dlg = std::make_shared<FindSuccessDlg>(this);
        std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg)->SetSearchInfo(si);
    }


    _find_dlg->show();
}
