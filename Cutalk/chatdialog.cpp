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

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog)
    , _mode(ChatUIMode::ChatMode)
    , _state(ChatUIMode::ChatMode)
    , _b_loading(false)
{
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
    // 创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i]);
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
        addChatUserList();
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
    //设置
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
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
        ui->contact_list->addContactUserList();
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
    //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
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
    //qDebug()<<"chat_user_wid sizeHint" << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_rsp->_uid, item);
}
