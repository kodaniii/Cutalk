#include "applyfriendpage.h"
#include "ui_applyfriendpage.h"
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QRandomGenerator>
#include "applyfrienditem.h"
#include "applyfriend.h"
#include "tcpmgr.h"
#include "usermgr.h"
#include "authenfriend.h"

ApplyFriendPage::ApplyFriendPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ApplyFriendPage){

    ui->setupUi(this);
    connect(ui->apply_friend_list, &ApplyFriendList::sig_show_search, this, &ApplyFriendPage::sig_show_search);
    initApplyList();
    //接受tcp传递的authrsp信号处理
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp, this, &ApplyFriendPage::slot_auth_rsp);

    //接受tcp传递的authrsp信号处理，对于双方已经是好友的情况，只需要将同意添加好友按钮置为已同意
    //发生的情况是：双方相互发好友申请，然后有一方接受了好友申请，对方没接受的情况
    //这里先这么做，后面直接修改Mysql，把双方的申请都置1，做双重判断
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp_set_btn_false, this, &ApplyFriendPage::slot_auth_rsp_set_btn_false);

}

ApplyFriendPage::~ApplyFriendPage()
{
    delete ui;
}


void ApplyFriendPage::AddNewApply(std::shared_ptr<AddFriendApply> apply)
{
    qDebug() << "ApplyFriendPage::AddNewApply()";
    /*新的好友添加申请，添加一个*/
    //int randomValue = QRandomGenerator::global()->bounded(100);
    int randomValue = apply->_from_uid;
    int head_i = randomValue % heads.size();
    auto* apply_item = new ApplyFriendItem();
    auto apply_info = std::make_shared<ApplyInfo>(apply->_from_uid,
                                                  apply->_name,
                                                  apply->_desc,
                                                  heads[head_i],
                                                  apply->_name, 0, 0);
    apply_item->SetInfo(apply_info);

    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    item->setSizeHint(apply_item->sizeHint());
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
    ui->apply_friend_list->insertItem(0, item);
    ui->apply_friend_list->setItemWidget(item, apply_item);
    apply_item->ShowAddBtn(true);

    /*点击item对应的同意好友申请按钮*/
    connect(apply_item, &ApplyFriendItem::sig_auth_friend, [this](std::shared_ptr<ApplyInfo> apply_info) {
        auto* authFriend = new AuthenFriend(this);
        authFriend->setModal(true);
        authFriend->SetApplyInfo(apply_info);
        authFriend->show();
    });
}

void ApplyFriendPage::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ApplyFriendPage::initApplyList()
{
    //从UserMgr中找到好友申请列表，添加好友申请
    auto apply_list = UserMgr::GetInstance()->GetApplyList();
    //由于ChatServer访问数据库里面是逆序添加，这里也逆序一下
    for(auto iter = apply_list.rbegin(); iter != apply_list.rend(); ++iter){
    //for(auto &apply: apply_list){
        auto apply = *iter;
        //int randomValue = QRandomGenerator::global()->bounded(100);
        int randomValue = apply->_uid;
        //头像没做，随机生成
        int head_i = randomValue % heads.size();

        auto* apply_item = new ApplyFriendItem();
        apply->SetIcon(heads[head_i]);
        apply_item->SetInfo(apply);

        QListWidgetItem* item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
        item->setSizeHint(apply_item->sizeHint());
        //不可与用户交互、不可选中
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);

        ui->apply_friend_list->insertItem(0, item); //头插
        ui->apply_friend_list->setItemWidget(item, apply_item);

        if(apply->_status){
            //已添加好友的情况
            //qDebug() << "apply_item->ShowAddBtn(false)";
            apply_item->ShowAddBtn(false);
        }else{
            //qDebug() << "apply_item->ShowAddBtn(true)";
            apply_item->ShowAddBtn(true);
            auto uid = apply_item->GetUid();
            //未添加的好友存储到内存
            _unauth_items[uid] = apply_item;
        }

        /*点击item对应的同意好友申请按钮*/
        connect(apply_item, &ApplyFriendItem::sig_auth_friend, [this](std::shared_ptr<ApplyInfo> apply_info) {
            auto* authFriend = new AuthenFriend(this);
            authFriend->setModal(true);
            authFriend->SetApplyInfo(apply_info);
            authFriend->show();
        });
    }

    /*测试：生成好友申请的item*/
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100);
        int str_i = randomValue%apply_friend_strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();
        bool status = 1 == randomValue % 2;

        auto *apply_item = new ApplyFriendItem();
        auto apply = std::make_shared<ApplyInfo>(0, names[name_i], apply_friend_strs[str_i],
                                                 heads[head_i], names[name_i], 0, status);
        apply_item->SetInfo(apply);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
        item->setSizeHint(apply_item->sizeHint());
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
        ui->apply_friend_list->addItem(item);
        ui->apply_friend_list->setItemWidget(item, apply_item);

        if(apply->_status){
            //已添加好友的情况
            //qDebug() << "apply_item->ShowAddBtn(false)";
            apply_item->ShowAddBtn(false);
        }else{
            //qDebug() << "apply_item->ShowAddBtn(true)";
            apply_item->ShowAddBtn(true);
            auto uid = apply_item->GetUid();
            //未添加的好友存储到内存
            _unauth_items[uid] = apply_item;
        }

        /*点击item对应的同意好友申请按钮*/
        connect(apply_item, &ApplyFriendItem::sig_auth_friend, [this](std::shared_ptr<ApplyInfo> apply_info){
            // auto *authFriend = new AuthenFriend(this);
            // authFriend->setModal(true);
            // authFriend->SetApplyInfo(apply_info);
            // authFriend->show();
            qDebug() << "This item just for show, cannot clickable";
        });
    }
}

void ApplyFriendPage::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp) {
    qDebug() << "ApplyFriendPage::slot_auth_rsp()";
    auto uid = auth_rsp->_uid;
    auto find_iter = _unauth_items.find(uid);
    if (find_iter == _unauth_items.end()) {
        return;
    }

    find_iter->second->ShowAddBtn(false);
}

void ApplyFriendPage::slot_auth_rsp_set_btn_false(std::shared_ptr<FriendInfo> info) {
    qDebug() << "ApplyFriendPage::slot_auth_rsp_set_btn_false()";
    auto uid = info->_uid;
    auto find_iter = _unauth_items.find(uid);
    if (find_iter == _unauth_items.end()) {
        return;
    }

    find_iter->second->ShowAddBtn(false);
}

