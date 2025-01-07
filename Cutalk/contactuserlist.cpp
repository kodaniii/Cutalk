#include "contactuserlist.h"
#include "defs.h"
#include "listitembase.h"
#include "grouptipitem.h"   //中间栏搜索栏的分组处，如“新的朋友”、“联系人”
#include "conuseritem.h"    //对于不同分组的列表项
#include <QRandomGenerator>
#include "tcpmgr.h"
#include "usermgr.h"
#include "userdata.h"
#include <QTimer>
#include <QCoreApplication>


ContactUserList::ContactUserList(QWidget *parent):
    _load_pending(false),
    _add_friend_item(nullptr)
{
    Q_UNUSED(parent);
    //默认隐藏水平和垂直滚动条
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //安装事件过滤器
    this->viewport()->installEventFilter(this);

    /*测试：模拟从数据库或后端传输过来的数据，加载各列表项，包括联系人、新朋友请求列表项*/
    initContactUserList();

    //点击分组下面的列表项，会根据列表项属于不同分组（好友申请列表、联系人列表）触发不同显示页面
    connect(this, &QListWidget::itemClicked, this, &ContactUserList::slot_item_clicked);

    //我方主动发送好友添加申请，对方同意后的回包处理，刷新联系人和聊天界面，这里处理联系人
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_auth_friend, this, &ContactUserList::slot_add_auth_firend);

    //我方同意其他人的好友添加申请后，我方需要刷新联系人和聊天界面，这里处理联系人
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp, this, &ContactUserList::slot_auth_rsp);
}

void ContactUserList::ShowRedPoint(bool bshow /*= true*/)
{
    _add_friend_item->ShowRedPoint(bshow);
}

void ContactUserList::initContactUserList()
{
    qDebug() << "ContactUserList::initContactUserList()";
    /*好友申请分组*/
    auto *groupTip = new GroupTipItem();
    groupTip->SetGroupTip(tr("好友申请"));
    //QlistWidgetItem容纳GroupTipItem
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(groupTip->sizeHint());
    this->addItem(item);
    //将groupTip分组控件设置为Widget item的子控件
    this->setItemWidget(item, groupTip);
    //不可被选中
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    /*申请好友分组下的好友申请项，这里调用ConUserItem，反正都是分组的项*/
    _add_friend_item = new ConUserItem();
    _add_friend_item->setObjectName("new_friend_item");
    _add_friend_item->SetInfo(0, tr("好友申请"), ":/res/add_friend.png");
    _add_friend_item->SetItemType(ListItemType::APPLY_FRIEND_ITEM);

    QListWidgetItem *add_item = new QListWidgetItem;
    //qDebug() << "chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    add_item->setSizeHint(_add_friend_item->sizeHint());
    this->addItem(add_item);
    this->setItemWidget(add_item, _add_friend_item);

    //默认设置好友申请项被选中
    this->setCurrentItem(add_item);

    /*联系人分组*/
    auto *groupCon = new GroupTipItem();
    groupCon->SetGroupTip(tr("联系人"));
    _groupitem = new QListWidgetItem;
    _groupitem->setSizeHint(groupCon->sizeHint());
    this->addItem(_groupitem);
    this->setItemWidget(_groupitem, groupCon);
    //不可选中
    _groupitem->setFlags(_groupitem->flags() & ~Qt::ItemIsSelectable);

    //加载后端发送过来的好友列表
    auto con_list = UserMgr::GetInstance()->GetConListPerPage();
    for(auto& con_ele : con_list){
        qDebug() << " -> contact ele uid" << con_ele->_uid
                 << "name" << con_ele->_name
                 << "desc" << con_ele->_desc
                 << "nick" << con_ele->_nick
                 << "sex" << con_ele->_sex
                 << "icon" << con_ele->_icon
                 << "back" << con_ele->_back;
        auto *con_user_wid = new ConUserItem();
        con_user_wid->SetInfo(con_ele->_uid, con_ele->_name, con_ele->_icon);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug() << "chat_user_wid sizeHint" << chat_user_wid->sizeHint();
        item->setSizeHint(con_user_wid->sizeHint());
        this->addItem(item);
        this->setItemWidget(item, con_user_wid);
    }
    //更新已加载条目
    UserMgr::GetInstance()->UpdateContactLoadedCount();

    /*测试：模拟生成联系人项*/
    addContactUserList();
}


void ContactUserList::addContactUserList()
{
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        //好友界面模拟出来的uid从18000起
        //这里模拟uid是因为对于不同好友，要识别好友的uid来选择对应的聊天项，或新建聊天项
        int uid = 18000 + i;
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *con_user_wid = new ConUserItem();
        con_user_wid->SetInfo(uid, names[name_i], heads[head_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
        item->setSizeHint(con_user_wid->sizeHint());
        this->addItem(item);
        this->setItemWidget(item, con_user_wid);
    }
}

bool ContactUserList::eventFilter(QObject *watched, QEvent *event)
{
    //检查事件是否是鼠标悬浮进入或离开
    if (watched == this->viewport()) {
        if (event->type() == QEvent::Enter) {
            //鼠标悬浮，显示滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
            //鼠标离开，隐藏滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    // 检查事件是否是鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // 计算滚动步数

        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

        // 检查是否滚动到底部
        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        //int pageSize = 10; // 每页加载的联系人数量

        if (maxScrollValue - currentValue <= 0) {
            // 滚动到底部，加载新的聯係人
            //qDebug() << "currentValue <= maxScrollValue, load more content";

            auto b_loaded = UserMgr::GetInstance()->IsLoadConFin();
            //已经加载完所有好友项，return
            // if(b_loaded){
            //     qDebug() << "b_loaded" << (b_loaded? "true": "false");
            //     return true;
            // }

            //发送信号通知聊天界面加载更多聊天内容
            emit sig_loading_contact_user();
        }

        //停止事件传递
        return true;
    }

    return QListWidget::eventFilter(watched, event);

}

void ContactUserList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item); // 获取自定义widget对象
    if(!widget){
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    // 对自定义widget进行操作， 将item转化为子类ListItemBase，为了调用GetItemType()
    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    //不处理分组item和无效item
    if(itemType == ListItemType::INVALID_ITEM || itemType == ListItemType::GROUP_TIP_ITEM){
        qDebug() << "slot invalid item clicked";
        return;
    }

    //处理好友请求item
    if(itemType == ListItemType::APPLY_FRIEND_ITEM){
        //创建对话框，提示用户
        qDebug() << "apply friend item clicked";
        //切换最右侧页面为好友申请列表
        emit sig_switch_apply_friend_page();
        return;
    }

    //联系人item
    if(itemType == ListItemType::CONTACT_USER_ITEM){
        //创建对话框，提示用户
        qDebug() << "contact user item clicked";
        //获取联系人item的userinfo信息
        auto con_item = qobject_cast<ConUserItem*>(customItem);
        auto user_info = con_item->GetInfo();

        //点击联系人，切换最右侧页面为该好友详细信息
        emit sig_switch_friend_info_page(user_info);
        return;
    }
}

void ContactUserList::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    qDebug() << "ContactUserList::slot_auth_rsp()";

    // bool isFriend = UserMgr::GetInstance()->CheckFriendById(auth_rsp->_uid);
    // //如果已经是好友了，添加好友无效
    // if(isFriend){
    //     return;
    // }

    //在联系人分组gourpitem之后插入新项
    //int randomValue = QRandomGenerator::global()->bounded(100);
    int randomValue = auth_rsp->_uid;
    int str_i = randomValue%strs.size();
    int head_i = randomValue%heads.size();

    auto *con_user_wid = new ConUserItem();
    con_user_wid->SetInfo(auth_rsp->_uid, auth_rsp->_name, heads[head_i]);
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    item->setSizeHint(con_user_wid->sizeHint());

    int index = this->row(_groupitem);
    this->insertItem(index + 1, item);

    this->setItemWidget(item, con_user_wid);
}

void ContactUserList::slot_add_auth_firend(std::shared_ptr<FriendInfo> auth_info)
{
    qDebug() << "ContactUserList::slot_add_auth_firend()";

    // bool isFriend = UserMgr::GetInstance()->CheckFriendById(auth_info->_uid);
    // //如果已经是好友了，添加好友无效
    // if(isFriend){
    //     return;
    // }

    //在联系人分组gourpitem之后插入新项
    //int randomValue = QRandomGenerator::global()->bounded(100);
    int randomValue = auth_info->_uid;
    int str_i = randomValue%strs.size();
    int head_i = randomValue%heads.size();

    auto *con_user_wid = new ConUserItem();
    con_user_wid->SetInfo(auth_info);
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    item->setSizeHint(con_user_wid->sizeHint());

    int index = this->row(_groupitem);
    this->insertItem(index + 1, item);

    this->setItemWidget(item, con_user_wid);
}


