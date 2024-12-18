#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QAction>
#include <QRandomGenerator>
#include "chatuserwid.h"
#include <QTimer>
#include <QMovie>

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

    ShowSearch(false);
    addChatUserList();
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::addChatUserList()
{
    std::vector<QString> strs = {
        "Hello, World!",
        "C++ is the best",
        "吃饭了吗？",
        "作业借我抄抄",
        "转让半盒热辣香骨鸡，肯德基疯狂星期四19.9入的，才吃了5块，里面还有10块，不舍得吃的时候就拿出来闻一闻，平常都是放在冰箱里，吃的时候就用公司的微波炉热一下，吃完都用钉书机钉起来，防止受潮。外表大概8成新吧。量很足，厚度约3cm，长度约6cm，包行货，不是外面的那种华莱士的，假一罚十，还有2盒甜辣酱一起送了，真的很好吃，平时小半块就可以回味半天了，价格私聊，非诚勿扰",
        "Welcome to the new era of communication!",
        "Let's embrace the challenges ahead.",
        "Together we can achieve great things.",
        "Stay focused on your goals, and you'll succeed.",
        "Every line of code is a step towards mastery.",
        "Innovation is the key to a brighter future.",
        "Knowledge is power, and sharing it is a virtue.",
        "The world is full of possibilities, just waiting to be discovered."
    };

    std::vector<QString> heads = {
        ":/res/head/head_1.jpg",
        ":/res/head/head_2.jpg",
        ":/res/head/head_3.jpg",
        ":/res/head/head_4.jpg",
        ":/res/head/head_5.jpg",
        ":/res/head/head_6.jpg",
        ":/res/head/head_7.jpg",
        ":/res/head/head_8.jpg",
        ":/res/head/head_9.jpg",
        ":/res/head/head_10.jpg",
        ":/res/head/head_11.jpg",
        ":/res/head/head_12.jpg",
        ":/res/head/head_13.jpg",
        ":/res/head/head_14.jpg",
        ":/res/head/head_15.jpg",
        ":/res/head/head_16.jpg",
        ":/res/head/head_17.jpg"
    };

    std::vector<QString> names = {
        "alice",
        "bob",
        "charlie",
        "diana",
        "eve",
        "frank",
        "grace",
        "hank"
    };

    // 创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug() << "chat_user_wid sizeHint is" << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
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
