#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QAction>
#include <QRandomGenerator>
#include "chatuserwid.h"
#include <QTimer>
#include <QMovie>
#include <QMouseEvent>
#include "tcpmgr.h"
#include "usermgr.h"
#include "conuseritem.h"

ChatDialog::ChatDialog(QWidget *parent):
    QDialog(parent),
    ui(new Ui::ChatDialog),
    _mode(ChatUIMode::ChatMode),
    _state(ChatUIMode::ChatMode),
    _b_loading(false),
    _cur_chat_uid(0),
    _last_widget(nullptr){

    ui->setupUi(this);

    //setWindowFlags(Qt::FramelessWindowHint | windowFlags()); // 设置无边框


    ui->add_btn->init("normal", "hover", "press");
    ui->search_edit->SetMaxLength(18);

    //用户名搜索位置添加搜索图标和搜索字样
    QAction *searchAction = new QAction(ui->search_edit);
    searchAction->setIcon(QIcon(":/res/search.png"));
    ui->search_edit->addAction(searchAction, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));

    //创建清除动作及图标
    QAction *clearAction = new QAction(ui->search_edit);
    clearAction->setIcon(QIcon(":/res/close_transparent.png"));
    //初始时不显示清除图标，将清除动作添加到LineEdit的末尾位置
    ui->search_edit->addAction(clearAction, QLineEdit::TrailingPosition);

    //当文本非空，需要显示清除图标
    connect(ui->search_edit, &QLineEdit::textChanged, [clearAction](const QString &text) {
        if (!text.isEmpty()) {
            clearAction->setIcon(QIcon(":/res/close_search.png"));
        }
        else {
            clearAction->setIcon(QIcon(":/res/close_transparent.png"));
        }
    });

    //清除文本
    connect(clearAction, &QAction::triggered, [this, clearAction]() {
        ui->search_edit->clear();
        clearAction->setIcon(QIcon(":/res/close_transparent.png"));
        ui->search_edit->clearFocus();
        //关闭搜索框
        ShowSearch(false);
    });

    //加载更多联系人用户列表
    connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user,
            this, &ChatDialog::slot_loading_chat_user);


    QPixmap pixmap(":/res/head/head_1.jpg");
    ui->side_head_lb->setPixmap(pixmap); // 将图片设置到QLabel上
    QPixmap scaledPixmap = pixmap.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio); // 将图片缩放到label的大小
    ui->side_head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    ui->side_head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小


    ui->side_chat_widget->init("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");

    //init会将state默认置为normal
    ui->side_chat_widget->setProperty("state","selected_pressed");
    //一定要修改curstate，否则会被hover悬停操作直接还原成unselected
    ui->side_chat_widget->SetCurState(LabelClickState::Selected);
    //和上一行同功能，之前写的时候没发现
    //ui->side_chat_widget->SetSelected(true);

    ui->side_contact_widget->init("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");


    addSideGroup(ui->side_chat_widget);
    addSideGroup(ui->side_contact_widget);

    connect(ui->side_chat_widget, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact_widget, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);

    connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_search_change);

    ShowSearch(false);
    addChatUserList();

    //安装事件过滤器
    //检测鼠标点击位置判断是否需要清空搜索框
    this->installEventFilter(this);


    connect(ui->contact_list, &ContactUserList::sig_loading_contact_user,
            this, &ChatDialog::slot_loading_contact_user);

    /*点击好友申请列表后，搜索界面关闭，并清空搜索框*/
    connect(ui->friend_apply_page, &ApplyFriendPage::sig_show_search,
            this, &ChatDialog::slot_show_search);

    //将search_edit拷贝给search_list，方便search_list获取search_edit的文本内容
    ui->search_list->SetSearchEdit(ui->search_edit);

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_notify_friend_apply, this, &ChatDialog::slot_notify_apply_friend);

    //我方主动发送好友添加申请，对方同意后的回包处理，刷新联系人和聊天界面，这里处理聊天
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_auth_friend, this, &ChatDialog::slot_add_auth_friend);

    //我方同意其他人的好友添加申请后，我方需要刷新联系人和聊天界面，这里处理聊天
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp, this, &ChatDialog::slot_auth_rsp);

    //搜索的用户是自己的好友时，跳转到和该好友的聊天item项
    connect(ui->search_list, &SearchList::sig_jump_chat_item, this, &ChatDialog::slot_jump_chat_item);

    //联系人列表中点击好友item跳转friend_info页面
    connect(ui->contact_list, &ContactUserList::sig_switch_friend_info_page,
            this, &ChatDialog::slot_friend_info_page);

    //点击联系人->好友申请，跳转到好友申请页面
    connect(ui->contact_list, &ContactUserList::sig_switch_apply_friend_page,
            this, &ChatDialog::slot_switch_apply_friend_page);

    //好友信息界面点击发送消息图标按钮，跳转到聊天界面
    connect(ui->friend_info_page, &FriendInfoPage::sig_jump_chat_item, this,
            &ChatDialog::slot_jump_chat_item_from_infopage);

    ui->stackedWidget->setCurrentWidget(ui->chat_page);

    //聊天列表项item点击
    connect(ui->chat_user_list, &QListWidget::itemClicked, this, &ChatDialog::slot_item_clicked);

    SetSelectChatItem();
    SetSelectChatPage();

    //在chatpage发送消息的同时，保存聊天记录到当前聊天用户recv_uid中，防止切换的时候聊天记录丢失
    connect(ui->chat_page, &ChatPage::sig_append_send_chat_msg, this, &ChatDialog::slot_append_send_chat_msg);

    //对方发送消息，本地保存当前聊天用户ChatUserWid::_user_info的聊天记录，并更新最后一次聊天内容
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_text_chat_msg, this, &ChatDialog::slot_text_chat_msg);

    //有一方发送消息，将正在聊天的聊天项置顶
    connect(this, &ChatDialog::sig_chat_item_to_top, this, &ChatDialog::slot_chat_item_to_top);
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        qDebug() << "ChatDialog::eventFilter() QEvent MouseButtonPress";
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        handleGlobalMousePress(mouseEvent);
    }
    return QDialog::eventFilter(watched, event);
}

void ChatDialog::handleGlobalMousePress(QMouseEvent *event)
{
    // 实现点击位置的判断和处理逻辑
    // 先判断是否处于搜索模式，如果不处于搜索模式则直接返回
    if( _mode != ChatUIMode::SearchMode){
        return;
    }

    // 将鼠标点击位置转换为搜索列表坐标系中的位置
    QPoint posInSearchList = ui->search_list->mapFromGlobal(event->globalPos());
    // 判断点击位置是否在聊天列表的范围内
    if (!ui->search_list->rect().contains(posInSearchList)) {
        // 如果不在聊天列表内，清空输入框
        ui->search_edit->clear();
        ShowSearch(false);
    }
}

void ChatDialog::addChatUserList()
{
    qDebug() << "ChatDialog::addChatUserList()";
    //先按照好友列表加载聊天记录，UserMgr::GetChatListPerPage()里面也暂时写的好友列表
    //TODO 按照聊天记录mysql表实现聊天记录加载
    auto friend_list = UserMgr::GetInstance()->GetChatListPerPage();
    if (friend_list.empty() == false) {
        for(auto& friend_ele : friend_list){
            qDebug() << " -> friend ele uid" << friend_ele->_uid
                     << "name" << friend_ele->_name
                     << "desc" << friend_ele->_desc
                     << "nick" << friend_ele->_nick
                     << "sex" << friend_ele->_sex
                     << "icon" << friend_ele->_icon
                     << "back" << friend_ele->_back;
            auto find_iter = _chat_items_added.find(friend_ele->_uid);
            if(find_iter != _chat_items_added.end()){
                continue;
            }
            auto *chat_user_wid = new ChatUserWid();
            auto user_info = std::make_shared<UserInfo>(friend_ele);
            chat_user_wid->SetInfo(user_info);
            QListWidgetItem *item = new QListWidgetItem;
            //qDebug() << "chat_user_wid sizeHint" << chat_user_wid->sizeHint();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item, chat_user_wid);
            _chat_items_added.insert(friend_ele->_uid, item);
        }

        //更新已加载条目
        UserMgr::GetInstance()->UpdateChatLoadedCount();
    }


    /*测试*/
    // 创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数

        int uid = 17000 + i;    //聊天界面模拟出来的uid从17000起
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *chat_user_wid = new ChatUserWid();
        auto user_info = std::make_shared<UserInfo>(uid, names[name_i], "", heads[head_i], 0, strs[str_i]);
        chat_user_wid->SetInfo(user_info);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug() << "chat_user_wid sizeHint" << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
}

void ChatDialog::addSideGroup(StateWidget *sw)
{
    _side_list.push_back(sw);
}

void ChatDialog::ShowSearch(bool b_search)
{
    //搜索模式，只显示搜索框
    if(b_search){
        ui->chat_user_list->hide();
        ui->contact_list->hide();
        ui->search_list->show();
        _mode = ChatUIMode::SearchMode;
    }
    //聊天模式，显示聊天列表
    else if(_state == ChatUIMode::ChatMode){
        ui->chat_user_list->show();
        ui->contact_list->hide();
        ui->search_list->hide();
        _mode = ChatUIMode::ChatMode;
    }
    //联系人模式，显示联系人
    else if(_state == ChatUIMode::ContactMode){
        ui->chat_user_list->hide();
        ui->search_list->hide();
        ui->contact_list->show();
        _mode = ChatUIMode::ContactMode;
    }
}

void ChatDialog::SetSelectChatItem(int uid)
{
    //没有聊天项，直接return
    if(ui->chat_user_list->count() <= 0){
        return;
    }

    //uid == 0: 当前没有选中任何聊天项
    if(uid == 0){
        //选中0行（第1行）
        ui->chat_user_list->setCurrentRow(0);
        //获取0行列表项
        QListWidgetItem *firstItem = ui->chat_user_list->item(0);
        if(!firstItem){
            return;
        }

        //获取其相关的widget
        QWidget *widget = ui->chat_user_list->itemWidget(firstItem);
        if(!widget){
            return;
        }

        //转为ChatUserWid用户聊天项widget
        auto chat_item = qobject_cast<ChatUserWid*>(widget);
        if(!chat_item){
            return;
        }

        _cur_chat_uid = chat_item->GetUserInfo()->_uid;

        return;
    }

    //从已添加聊天项中找到对应uid的项
    auto find_iter = _chat_items_added.find(uid);
    if(find_iter == _chat_items_added.end()){
        qDebug() << "Cannot found ChatItem uid" << uid << ", set current row 0";
        ui->chat_user_list->setCurrentRow(0);
        return;
    }

    ui->chat_user_list->setCurrentItem(find_iter.value());

    _cur_chat_uid = uid;
}

void ChatDialog::SetSelectChatPage(int uid)
{
    if (ui->chat_user_list->count() <= 0){
        return;
    }

    //当前没有选中任何聊天项，获取0行的信息
    if (uid == 0) {
        auto item = ui->chat_user_list->item(0);

        QWidget* widget = ui->chat_user_list->itemWidget(item);
        if (!widget) {
            return;
        }

        auto con_item = qobject_cast<ChatUserWid*>(widget);
        if (!con_item) {
            return;
        }

        //获取用户信息
        auto user_info = con_item->GetUserInfo();
        ui->chat_page->SetUserInfo(user_info);
        return;
    }

    auto find_iter = _chat_items_added.find(uid);
    if(find_iter == _chat_items_added.end()){
        return;
    }

    QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
    if(!widget){
        return;
    }

    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug()<< "qobject_cast<ListItemBase*>(widget) is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if(itemType == CHAT_USER_ITEM){
        auto con_item = qobject_cast<ChatUserWid*>(customItem);
        if(!con_item){
            return;
        }

        //设置用户信息
        auto user_info = con_item->GetUserInfo();
        ui->chat_page->SetUserInfo(user_info);

        return;
    }
}

void ChatDialog::loadMoreChatUser() {
    //暂时用好友替代
    auto friend_list = UserMgr::GetInstance()->GetChatListPerPage();
    if (friend_list.empty() == false) {
        for(auto& friend_ele : friend_list){
            auto find_iter = _chat_items_added.find(friend_ele->_uid);
            if(find_iter != _chat_items_added.end()){
                continue;
            }
            auto *chat_user_wid = new ChatUserWid();
            auto user_info = std::make_shared<UserInfo>(friend_ele);
            chat_user_wid->SetInfo(user_info);
            QListWidgetItem *item = new QListWidgetItem;
            //qDebug() << "chat_user_wid sizeHint" << chat_user_wid->sizeHint();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item, chat_user_wid);
            _chat_items_added.insert(friend_ele->_uid, item);
        }

        //更新已加载条目
        UserMgr::GetInstance()->UpdateChatLoadedCount();
    }
}

void ChatDialog::loadMoreConUser()
{
    qDebug() << "ChatDialog::loadMoreConUser()";
    auto friend_list = UserMgr::GetInstance()->GetConListPerPage();
    if (friend_list.empty() == false) {
        for(auto & friend_ele : friend_list){
            auto *chat_user_wid = new ConUserItem();
            chat_user_wid->SetInfo(friend_ele->_uid,friend_ele->_name,
                                   friend_ele->_icon);
            QListWidgetItem *item = new QListWidgetItem;
            //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->contact_list->addItem(item);
            ui->contact_list->setItemWidget(item, chat_user_wid);
        }

        //更新已加载条目
        UserMgr::GetInstance()->UpdateContactLoadedCount();
    }
}

void ChatDialog::UpdateChatMsg(std::vector<std::shared_ptr<TextChatData>> msgdata)
{
    for(auto& msg : msgdata){
        if(msg->_send_uid != _cur_chat_uid){
            break;
        }

        ui->chat_page->AppendChatMsg(msg);
    }
}

void ChatDialog::slot_loading_chat_user()
{
    if(_b_loading){
        return;
    }

    _b_loading = true;

    /*loading gif显示*/
    QLabel *loading_item = new QLabel(this);
    QMovie *movie=new QMovie(":/res/loading.gif");

    loading_item->setMovie(movie);
    loading_item->setFixedSize(250, 70);
    loading_item->setAlignment(Qt::AlignCenter);
    movie->setScaledSize(QSize(50, 50));


    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(QSize(250, 70));
    ui->chat_user_list->addItem(item);
    ui->chat_user_list->setItemWidget(item, loading_item);

    movie->start();

    QTimer::singleShot(550, this, [this, item](){
        qDebug() << "ChatDialog::slot_loading_chat_user()";
        //addChatUserList();
        loadMoreChatUser();
        ui->chat_user_list->takeItem(ui->chat_user_list->row(item));
        ui->chat_user_list->update();

        _b_loading = false;
    });

}

void ChatDialog::clearSideState(StateWidget *sw)
{
    for(auto & ele: _side_list){
        if(ele == sw){
            continue;
        }

        ele->ClearState();
    }
}

void ChatDialog::slot_side_chat()
{
    qDebug()<< "ChatDialog::slot_side_chat()";
    clearSideState(ui->side_chat_widget);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _state = ChatUIMode::ChatMode;
    ShowSearch(false);
}

void ChatDialog::slot_side_contact(){
    qDebug()<< "ChatDialog::slot_side_contact()";
    clearSideState(ui->side_contact_widget);
    if(_last_widget){
        //调用上次保存的_last_widget
        ui->stackedWidget->setCurrentWidget(_last_widget);
    }
    else{
        ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    }
    _state = ChatUIMode::ContactMode;
    ShowSearch(false);
}

void ChatDialog::slot_search_change(const QString &str)
{
    if (!str.isEmpty()) {
        ShowSearch(true);
    }
}

void ChatDialog::slot_loading_contact_user()
{

    if(_b_loading){
        return;
    }

    _b_loading = true;

    /*loading gif显示*/
    QLabel *loading_item = new QLabel(this);
    QMovie *movie=new QMovie(":/res/loading.gif");

    loading_item->setMovie(movie);
    loading_item->setFixedSize(250, 70);
    loading_item->setAlignment(Qt::AlignCenter);
    movie->setScaledSize(QSize(50, 50));


    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(QSize(250, 70));
    ui->contact_list->addItem(item);
    ui->contact_list->setItemWidget(item, loading_item);

    movie->start();

    QTimer::singleShot(550, this, [this, item](){
        qDebug() << "ChatDialog::slot_loading_contact_user()";
        //ui->contact_list->addContactUserList();
        loadMoreConUser();
        ui->contact_list->takeItem(ui->contact_list->row(item));
        ui->contact_list->update();

        _b_loading = false;
    });
}

void ChatDialog::slot_show_search(bool b_search)
{
    ui->search_edit->clear();
    ShowSearch(b_search);
}

void ChatDialog::slot_notify_apply_friend(std::shared_ptr<AddFriendApply> apply)
{
    qDebug() << "ChatDialog::slot_notify_apply_friend()";

    bool b_already = UserMgr::GetInstance()->AlreadyApply(apply->_from_uid);
    /*如果已经发送过好友申请，不处理*/
    if(b_already){
        qDebug() << "UserMgr::GetInstance()->AlreadyApply() true, return...";
        return;
    }


    UserMgr::GetInstance()->AddApplyList(std::make_shared<ApplyInfo>(apply));
    ui->side_contact_widget->ShowRedPoint(true);
    ui->contact_list->ShowRedPoint(true);
    ui->friend_apply_page->AddNewApply(apply);
}

void ChatDialog::slot_add_auth_friend(std::shared_ptr<FriendInfo> auth_info)
{
    qDebug() << "ChatDialog::slot_add_auth_friend()";
    qDebug() << "Auth_info uid" << auth_info->_uid
             << ", name" << auth_info->_name
             << ", nick" << auth_info->_nick;

    // //如果已经是好友，跳过
    // auto bfriend = UserMgr::GetInstance()->CheckFriendById(auth_info->_uid);
    // if(bfriend){
    //     return;
    // }

    UserMgr::GetInstance()->AddFriend(auth_info);

    //int randomValue = QRandomGenerator::global()->bounded(100);
    int randomValue = auth_info->_uid;
    int str_i = randomValue % strs.size();
    int head_i = randomValue % heads.size();
    int name_i = randomValue % names.size();

    auto *chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_info);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug() << "chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_info->_uid, item);
}

void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    qDebug() << "ChatDialog::slot_auth_rsp()";
    qDebug() << "Auth_info uid" << auth_rsp->_uid
             << ", name" << auth_rsp->_name
             << ", icon" << auth_rsp->_icon
             << ", nick" << auth_rsp->_nick;

    // //判断如果已经是好友则跳过
    // auto bfriend = UserMgr::GetInstance()->CheckFriendById(auth_rsp->_uid);
    // if(bfriend){
    //     return;
    // }

    // //int randomValue = QRandomGenerator::global()->bounded(100);
    // int randomValue = auth_rsp->_uid;
    // int str_i = randomValue % strs.size();
    // int head_i = randomValue % heads.size();
    // int name_i = randomValue % names.size();

    // /*生成默认头像，因为头像没上传，直接用默认头像*/
    // auth_rsp->_icon = head_i;

    auto *chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_rsp);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug() << "chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_rsp->_uid, item);
}

void ChatDialog::slot_jump_chat_item(std::shared_ptr<SearchInfo> si)
{
    qDebug() << "ChatDialog::slot_jump_chat_item()";
    auto find_iter = _chat_items_added.find(si->_uid);
    //聊天项还未加载
    if(find_iter != _chat_items_added.end()){
        ui->chat_user_list->scrollToItem(find_iter.value());
        ui->side_chat_widget->SetSelected(true);
        //设置聊天item为选中状态
        SetSelectChatItem(si->_uid);
        //更新聊天界面信息
        SetSelectChatPage(si->_uid);
        slot_side_chat();
        return;
    }

    //如果没找到，则创建新的插入listwidget
    auto* chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(si);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug() << "chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);

    _chat_items_added.insert(si->_uid, item);

    ui->side_chat_widget->SetSelected(true);
    //选中聊天项
    SetSelectChatItem(si->_uid);
    //更新聊天界面
    SetSelectChatPage(si->_uid);
    //切换聊天界面
    slot_side_chat();
}


void ChatDialog::slot_friend_info_page(std::shared_ptr<FriendInfo> user_info)
{
    qDebug() << "ChatDialog::slot_friend_info_page()";
    _last_widget = ui->friend_info_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_info_page);
    ui->friend_info_page->SetInfo(user_info);
}


void ChatDialog::slot_switch_apply_friend_page()
{
    qDebug() << "ChatDialog::slot_switch_apply_friend_page()";
    _last_widget = ui->friend_apply_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
}

void ChatDialog::slot_jump_chat_item_from_infopage(std::shared_ptr<FriendInfo> user_info)
{
    qDebug() << "ChatDialog::slot_jump_chat_item_from_infopage()";
    auto find_iter = _chat_items_added.find(user_info->_uid);
    if(find_iter != _chat_items_added.end()){
        qDebug() << "jump to chat item, uid" << user_info->_uid;
        ui->chat_user_list->scrollToItem(find_iter.value());
        ui->side_chat_widget->SetSelected(true);
        SetSelectChatItem(user_info->_uid);
        //更新聊天界面信息
        SetSelectChatPage(user_info->_uid);
        slot_side_chat();
        return;
    }

    //如果没找到，则创建新的插入listwidget
    auto* chat_user_wid = new ChatUserWid();
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);

    _chat_items_added.insert(user_info->_uid, item);

    ui->side_chat_widget->SetSelected(true);
    SetSelectChatItem(user_info->_uid);
    //更新聊天界面信息
    SetSelectChatPage(user_info->_uid);
    slot_side_chat();
}

void ChatDialog::slot_item_clicked(QListWidgetItem *item)
{
    qDebug() << "ChatDialog::slot_item_clicked()";
    //获取点击item对应的widget
    QWidget *widget = ui->chat_user_list->itemWidget(item);
    if(!widget){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    //将item转化为子类ListItemBase
    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if(itemType == ListItemType::INVALID_ITEM
        || itemType == ListItemType::GROUP_TIP_ITEM){
        qDebug() << "slot invalid item clicked";
        return;
    }


    if(itemType == ListItemType::CHAT_USER_ITEM){
        qDebug() << "contact user item clicked";

        auto chat_wid = qobject_cast<ChatUserWid*>(customItem);
        auto user_info = chat_wid->GetUserInfo();
        //跳转到聊天界面
        ui->chat_page->SetUserInfo(user_info);
        _cur_chat_uid = user_info->_uid;
        return;
    }
}

void ChatDialog::slot_append_send_chat_msg(std::shared_ptr<TextChatData> msgdata)
{
    qDebug() << "ChatDialog::slot_append_send_chat_msg()";
    if (_cur_chat_uid == 0) {
        return;
    }

    auto find_iter = _chat_items_added.find(_cur_chat_uid);
    if (find_iter == _chat_items_added.end()) {
        return;
    }

    QWidget* widget = ui->chat_user_list->itemWidget(find_iter.value());
    if (!widget) {
        return;
    }

    ListItemBase* customItem = qobject_cast<ListItemBase*>(widget);
    if (!customItem) {
        qDebug() << "qobject_cast<ListItemBase*>(widget) is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if (itemType == CHAT_USER_ITEM) {
        auto con_item = qobject_cast<ChatUserWid*>(customItem);
        if (!con_item) {
            return;
        }

        auto user_info = con_item->GetUserInfo();
        //拼接user_info的消息，切换聊天项的时候，chat_page会自动从这里读取消息
        //user_info->_chat_msgs.push_back(msgdata);

        //这里保存的是我方主动发送的消息
        std::vector<std::shared_ptr<TextChatData>> msg_vec;
        msg_vec.push_back(msgdata);
        UserMgr::GetInstance()->AppendFriendChatMsg(_cur_chat_uid, msg_vec);

        //更新聊天list当前uid项最后一次消息，因为是本方发，所以应该找到recv方的item
        auto find_iter = _chat_items_added.find(msgdata->_recv_uid);
        if(find_iter != _chat_items_added.end()){
            QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
            auto chat_wid = qobject_cast<ChatUserWid*>(widget);
            if(!chat_wid){
                return;
            }
            chat_wid->updateLastMsg(msg_vec);

            //return;
        }

        emit sig_chat_item_to_top(msgdata->_recv_uid, true);
        return;
    }
}

void ChatDialog::slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg)
{
    qDebug() << "ChatDialog::slot_text_chat_msg()";
    //查找是否已经有相关用户uid的聊天项
    auto find_iter = _chat_items_added.find(msg->_send_uid);

    //存在相关uid聊天项，将新传来的消息保存到聊天用户widget中，并刷新最后一次的内容
    if(find_iter != _chat_items_added.end()){
        qDebug() << "handle send_uid msg, uid" << msg->_send_uid;
        QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
        //聊天list的小横条
        auto chat_wid = qobject_cast<ChatUserWid*>(widget);
        if(!chat_wid){
            return;
        }
        //保存当前聊天用户_user_info的聊天记录，并在list的item聊天项中更新最后一次聊天内容
        chat_wid->updateLastMsg(msg->_chat_msgs);
        //显示当前用户聊天气泡及内容
        UpdateChatMsg(msg->_chat_msgs);
        UserMgr::GetInstance()->AppendFriendChatMsg(msg->_send_uid, msg->_chat_msgs);


        emit sig_chat_item_to_top(msg->_send_uid, false);
        return;
    }

    //如果没找到，则创建新的聊天项
    auto* chat_user_wid = new ChatUserWid();
    //查询好友信息
    auto fi_ptr = UserMgr::GetInstance()->GetFriendById(msg->_send_uid);
    chat_user_wid->SetInfo(fi_ptr);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug() << "chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    chat_user_wid->updateLastMsg(msg->_chat_msgs);
    UserMgr::GetInstance()->AppendFriendChatMsg(msg->_send_uid, msg->_chat_msgs);
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(msg->_send_uid, item);
    emit sig_chat_item_to_top(msg->_send_uid, false);

    return;
}

void ChatDialog::slot_chat_item_to_top(int uid, bool isSend)
{
    qDebug() << "ChatDialog::slot_chat_item_to_top() uid" << uid;
    // 查找当前聊天项
    auto find_iter = _chat_items_added.find(uid);
    if (find_iter == _chat_items_added.end()) {
        return;
    }

    // 获取当前聊天项的QListWidgetItem
    QListWidgetItem* oldItem = find_iter.value();
    if (!oldItem) {
        return;
    }

    // 获取与oldItem关联的自定义控件
    QWidget* widget = ui->chat_user_list->itemWidget(oldItem);
    if (!widget) {
        qDebug() << "Widget is nullptr";
        return;
    }

    qDebug() << "ui->chat_user_list->row(oldItem)" << ui->chat_user_list->row(oldItem);
    // 对于接收者，如果接收者正在聊天的对象就是对方，不需要额外处理，如果处理，会选中其他对象
    if (!isSend && ui->chat_user_list->row(oldItem) == 0) {
        return;
    }

    // 创建一个新的QListWidgetItem
    QListWidgetItem* newItem = new QListWidgetItem();
    newItem->setSizeHint(widget->sizeHint());

    // 将新的item插入到列表的最顶部
    ui->chat_user_list->insertItem(0, newItem);

    // 将自定义控件设置到新的item上
    ui->chat_user_list->setItemWidget(newItem, widget);

    // 更新_map中的item引用
    _chat_items_added[uid] = newItem;

    // 删除原来的QListWidgetItem
    delete oldItem;

    // 如果是我方发送消息，还需要重新选中第一条
    if(isSend){
        ui->chat_user_list->setCurrentItem(newItem);
        qDebug() << "ChatDialog::slot_chat_item_to_top() succ";
    }
}
