#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QAction>
#include <QRandomGenerator>
#include "chatuserwid.h"

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog)
    , _mode(ChatUIMode::ChatMode)
    , _state(ChatUIMode::ChatMode)
    , _b_loading(false)
{
    ui->setupUi(this);

    ui->add_btn->SetState("normal", "hover", "press");
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
