#include "applyfriend.h"
#include "ui_applyfriend.h"
#include "clicklabel.h"
#include "friendlabel.h"
#include <QScrollBar>
#include "usermgr.h"
#include "tcpmgr.h"
#include <QJsonDocument>

ApplyFriend::ApplyFriend(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ApplyFriend)
    , _label_point(2,6) {

    qDebug() << "ApplyFriend::ApplyFriend()";
    ui->setupUi(this);
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("ApplyFriend");
    this->setModal(true);

    //未输入信息时的提示
    ui->reason_ed->setPlaceholderText(tr("在此处输入好友申请内容..."));
    ui->lb_ed->setPlaceholderText("搜索/添加标签");
    ui->back_ed->setPlaceholderText("添加该好友的备注");



    //ui->lb_ed->SetMaxLength(21);
    ui->lb_ed->move(2, 2);
    //ui->lb_ed->setFixedHeight(20);
    ui->lb_ed->setMaxLength(16);

    //输入标签后，会有对应的提示框，先隐藏
    ui->input_tip_wid->hide();

    _tip_cur_point = QPoint(5, 5);

    /*测试：模拟已经存在的标签*/    
    _tip_data = {"集合啦！动物森友会", "塞尔达传说：荒野之息", "超级马里奥奥德赛", "怪物猎人：世界",
                 "最终幻想14", "星之卡比：新星同盟", "任天堂明星大乱斗", "火焰纹章：风花雪月", "异度之刃2"};
    //空标签测试
    //_tip_data = {};

    connect(ui->more_lb, &ClickOnceLabel::clicked, this, &ApplyFriend::ShowMoreLabel);
    InitTipLbs();
    //链接输入标签回车事件
    connect(ui->lb_ed, &CustomizeEdit::returnPressed, this, &ApplyFriend::SlotLabelEnter);
    connect(ui->lb_ed, &CustomizeEdit::textChanged, this, &ApplyFriend::SlotLabelTextChange);
    connect(ui->lb_ed, &CustomizeEdit::editingFinished, this, &ApplyFriend::SlotLabelEditFinished);
    connect(ui->tip_lb, &ClickOnceLabel::clicked, this, &ApplyFriend::SlotAddFirendLabelByClickTip);

    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);
    ui->sure_btn->init("normal","hover","press");
    ui->cancel_btn->init("normal","hover","press");
    //连接确认和取消按钮的槽函数
    connect(ui->cancel_btn, &QPushButton::clicked, this, &ApplyFriend::SlotApplyCancel);
    connect(ui->sure_btn, &QPushButton::clicked, this, &ApplyFriend::SlotApplySure);


    //ui->lb_list->setFixedHeight(65);
}

ApplyFriend::~ApplyFriend()
{
    qDebug() << "ApplyFriend::~ApplyFriend()";
    delete ui;
}


void ApplyFriend::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        m_isDragging = true;
    }
}

void ApplyFriend::mouseMoveEvent(QMouseEvent *event)
{
    /*修复聊天窗口侧边栏移动过小导致窗口跳动的BUG
    有效果，但还不是完全有效果，所以这里代码也没删，
    在statewidget中将mousePressEvent和mouseMoveEvent做了忽略
    */
    if (event->buttons() & Qt::LeftButton && m_isDragging)
    {
        // 计算新的位置
        QPoint newPos = event->globalPos() - m_dragPosition;

        // 计算当前位置与新位置之间的距离
        int distance = (newPos - this->pos()).manhattanLength();

        // 设置一个阈值，只有当移动距离大于阈值时才移动窗口
        const int threshold = 5;
        if (distance > threshold)
        {
            move(newPos);
            //qDebug() << "move to" << newPos;
        }
    }
    /*
    if (event->buttons() & Qt::LeftButton && m_isDragging)
    {
        move(event->globalPos() - m_dragPosition);
    }*/
}

void ApplyFriend::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDragging = false;
    }
}

void ApplyFriend::InitTipLbs()
{
    qDebug() << "ApplyFriend::InitTipLbs()";
    int lines = 1;
    for(int i = 0; i < _tip_data.size(); i++){

        auto* lb = new ClickLabel(ui->lb_list);
        lb->init("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()) {
            lines++;
            if (lines > 2) {
                delete lb;
                return;
            }

            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

        }

        auto next_point = _tip_cur_point;

        AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);

        _tip_cur_point = next_point;
    }

}


void ApplyFriend::AddTipLbs(ClickLabel* lb, QPoint cur_point, QPoint& next_point, int text_width, int text_height)
{
    qDebug() << "ApplyFriend::AddTipLbs()";
    lb->move(cur_point);
    lb->show();
    _add_labels.insert(lb->text(), lb);
    qDebug() << lb->text() << "move to" << cur_point;
    _add_label_keys.push_back(lb->text());
    next_point.setX(lb->pos().x() + text_width + 15);
    next_point.setY(lb->pos().y());
}

bool ApplyFriend::eventFilter(QObject *obj, QEvent *event)
{
    //qDebug() << "ApplyFriend::eventFilter()";
    if (obj == ui->scrollArea && event->type() == QEvent::Enter)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(false);
    }
    else if (obj == ui->scrollArea && event->type() == QEvent::Leave)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QObject::eventFilter(obj, event);
}

void ApplyFriend::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    qDebug() << "ApplyFriend::SetSearchInfo()";
    _si = si;

    //TcpMgr还没做相关的用户搜索逻辑
    //这里暂时代替的是TcpMgr::initHandlers() REQ_CHAT_LOGIN_RSP逻辑，目前只看界面效果
    /*
    auto applyname = UserMgr::GetInstance()->GetName();
    auto bakname = si->_name;
    ui->reason_ed->setText(applyname);
    ui->back_ed->setText(bakname);*/
    ui->reason_ed->setText("");
    ui->back_ed->setText("");
}

void ApplyFriend::ShowMoreLabel()
{
    qDebug()<< "ApplyFriend::ShowMoreLabel()";
    ui->more_lb_wid->hide();

    ui->lb_list->setFixedWidth(420);
    _tip_cur_point = QPoint(5, 5);
    auto next_point = _tip_cur_point;
    int textWidth;
    int textHeight;
    //重排现有的label
    for(auto & added_key : _add_label_keys){
        auto added_lb = _add_labels[added_key];

        QFontMetrics fontMetrics(added_lb->font()); // 获取QLabel控件的字体信息
        textWidth = fontMetrics.width(added_lb->text()); // 获取文本的宽度
        textHeight = fontMetrics.height(); // 获取文本的高度

        if(_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()){
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y()+textHeight+15);
        }
        added_lb->move(_tip_cur_point);

        next_point.setX(added_lb->pos().x() + textWidth + 15);
        next_point.setY(_tip_cur_point.y());

        _tip_cur_point = next_point;

    }

    //添加未添加的
    for(int i = 0; i < _tip_data.size(); i++){
        auto iter = _add_labels.find(_tip_data[i]);
        if(iter != _add_labels.end()){
            continue;
        }

        auto* lb = new ClickLabel(ui->lb_list);
        lb->init("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()) {

            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

        }

        next_point = _tip_cur_point;

        AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);

        _tip_cur_point = next_point;

    }

    int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    //qDebug()<<"after resize ui->lb_list size is" <<  ui->lb_list->size();
    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height()+diff_height);
}

void ApplyFriend::resetLabels()
{
    qDebug() << "ApplyFriend::resetLabels()";
    auto max_width = ui->gridWidget->width();
    qDebug() << "ui->gridWidget->width()" << ui->gridWidget->width();
    auto label_height = 0;
    for(auto iter = _friend_labels.begin(); iter != _friend_labels.end(); iter++){
        //todo... 添加宽度统计
        if( _label_point.x() + iter.value()->width() > max_width) {
            _label_point.setY(_label_point.y()+iter.value()->height()+6);
            _label_point.setX(2);
        }

        iter.value()->move(_label_point);
        iter.value()->show();

        _label_point.setX(_label_point.x()+iter.value()->width()+2);
        _label_point.setY(_label_point.y());
        label_height = iter.value()->height();
    }

    if(_friend_labels.isEmpty()){
        ui->lb_ed->move(_label_point);
        return;
    }

    if(_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()){
        ui->lb_ed->move(2,_label_point.y()+label_height+6);
    }else{
        ui->lb_ed->move(_label_point);
    }
}

void ApplyFriend::addLabel(QString name)
{
    qDebug() << "ApplyFriend::addLabel()";
    //判重，如果新添加的_friend_label已经存在，清空标签label
    if (_friend_labels.find(name) != _friend_labels.end()) {
        ui->lb_ed->clear();
        return;
    }

    //创建一个FriendLabel实例，由label和一个关闭按钮×组成
    auto tmplabel = new FriendLabel(ui->gridWidget);
    tmplabel->SetText(name);
    tmplabel->setObjectName("FriendLabel");

    //gridWidget宽度，便于后续位置计算
    auto max_width = ui->gridWidget->width();

    /*计算显示标签栏的标签位置*/
    //超过最大宽度，移动到下一行显示，重置x
    if (_label_point.x() + tmplabel->width() > max_width) {
        _label_point.setY(_label_point.y() + tmplabel->height() + 6);
        _label_point.setX(2);
    }
    else{
        //_label_point.setX(_label_point.x() + tmplabel->width() + 2);
    }

    tmplabel->move(_label_point);
    tmplabel->show();

    //存储到新添加的标签
    _friend_labels[tmplabel->Text()] = tmplabel;
    _friend_label_keys.push_back(tmplabel->Text());

    //点击关闭按钮，移除新添加的label
    connect(tmplabel, &FriendLabel::sig_close, this, &ApplyFriend::SlotRemoveFriendLabel);

    /*更新标签输入框的位置*/
    _label_point.setX(_label_point.x() + tmplabel->width() + 2);

    if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()) {
        ui->lb_ed->move(2, _label_point.y() + tmplabel->height() + 2);
    }
    else {
        ui->lb_ed->move(_label_point);
    }

    qDebug() << _label_point;

    //清除编辑框
    ui->lb_ed->clear();

    //如果添加标签后gridWidget高度不足以显示新标签，调整高度
    if (ui->gridWidget->height() < _label_point.y() + tmplabel->height() + 2) {
        ui->gridWidget->setFixedHeight(_label_point.y() + tmplabel->height() * 2 + 2);
    }
}

void ApplyFriend::SlotLabelEnter()
{
    qDebug() << "ApplyFriend::SlotLabelEnter()";
    //标签输入栏空
    if(ui->lb_ed->text().isEmpty()){
        return;
    }

    auto text = ui->lb_ed->text();

    addLabel(ui->lb_ed->text());

    //隐藏输入提示标签widget
    ui->input_tip_wid->hide();

    //从缓存中查找标签是不是已经添加过
    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    //没找到，添加
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    //判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(LabelClickState::Selected);
        return;
    }

    //标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickLabel(ui->lb_list);
    lb->init("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);
    qDebug() << "ui->lb_list->width()" << ui->lb_list->width();
    qDebug() << "_tip_cur_point.x()" << _tip_cur_point.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth" << textWidth;

    if (_tip_cur_point.x() + textWidth + tip_offset + 3 > ui->lb_list->width()) {

        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

    }

    auto next_point = _tip_cur_point;

    AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    lb->SetCurState(LabelClickState::Selected);

    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
}

void ApplyFriend::SlotRemoveFriendLabel(QString name)
{
    qDebug() << "ApplyFriend::SlotRemoveFriendLabel()";

    _label_point.setX(2);
    _label_point.setY(6);

    auto find_iter = _friend_labels.find(name);

    if(find_iter == _friend_labels.end()){
        return;
    }

    auto find_key = _friend_label_keys.end();
    for(auto iter = _friend_label_keys.begin(); iter != _friend_label_keys.end(); iter++){
        if(*iter == name){
            find_key = iter;
            break;
        }
    }

    if(find_key != _friend_label_keys.end()){
        _friend_label_keys.erase(find_key);
    }


    delete find_iter.value();

    _friend_labels.erase(find_iter);

    resetLabels();

    auto find_add = _add_labels.find(name);
    if(find_add == _add_labels.end()){
        return;
    }

    find_add.value()->ResetNormalState();
}

//点击标已有签添加或删除新联系人的标签
void ApplyFriend::SlotChangeFriendLabelByTip(QString lbtext, LabelClickState state)
{
    qDebug() << "ApplyFriend::SlotChangeFriendLabelByTip()";
    auto find_iter = _add_labels.find(lbtext);
    if(find_iter == _add_labels.end()){
        return;
    }

    if(state == LabelClickState::Selected){
        //添加
        addLabel(lbtext);
        return;
    }

    if(state == LabelClickState::Unselected){
        //删除
        SlotRemoveFriendLabel(lbtext);
        return;
    }

}

void ApplyFriend::SlotLabelTextChange(const QString& text)
{
    qDebug() << "ApplyFriend::SlotLabelTextChange()";
    if (text.isEmpty()) {
        ui->tip_lb->setText("");
        ui->input_tip_wid->hide();
        return;
    }

    auto iter = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (iter == _tip_data.end()) {
        auto new_text = add_prefix + text;
        ui->tip_lb->setText(new_text);
        ui->input_tip_wid->show();
        return;
    }
    ui->tip_lb->setText(text);
    ui->input_tip_wid->show();
}

void ApplyFriend::SlotLabelEditFinished()
{
    qDebug() << "ApplyFriend::SlotLabelEditFinished()";
    ui->input_tip_wid->hide();
}

void ApplyFriend::SlotAddFirendLabelByClickTip(QString text)
{
    qDebug() << "ApplyFriend::SlotAddFirendLabelByClickTip()";
    int index = text.indexOf(add_prefix);
    if (index != -1) {
        text = text.mid(index + add_prefix.length());
    }

    //不知道为什么点到空的位置会返回空字符串，并创建空的标签，这里做过滤
    if (text.isEmpty()){
        return;
    }
    addLabel(text);

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    //判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(LabelClickState::Selected);
        return;
    }

    //标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickLabel(ui->lb_list);
    lb->init("normal", "hover", "pressed", "selected_normal",
             "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);
    qDebug() << "ui->lb_list->width()" << ui->lb_list->width();
    qDebug() << "_tip_cur_point.x()" << _tip_cur_point.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth" << textWidth;

    if (_tip_cur_point.x() + textWidth + tip_offset + 3 > ui->lb_list->width()) {
        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

    }

    auto next_point = _tip_cur_point;

    AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();

    //加了之后标签栏输入多了后就不增加高度了
    //qDebug() << "ui->lb_list height" << ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);
    //qDebug() << "ui->lb_list height goto" << ui->lb_list->height();

    lb->SetCurState(LabelClickState::Selected);

    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height()+ diff_height);
}

void ApplyFriend::SlotApplyCancel()
{
    qDebug() << "ApplyFriend::SlotApplyCancel()";
    this->hide();
    deleteLater();
}

void ApplyFriend::SlotApplySure()
{
    qDebug() << "ApplyFriend::SlotApplySure()" ;

    //TODO 发送好友请求逻辑，is ok
    QJsonObject jsonObj;
    /*发送方信息*/
    auto uid = UserMgr::GetInstance()->GetUid();
    jsonObj["send_uid"] = uid;
    jsonObj["send_name"] = UserMgr::GetInstance()->GetName();
    auto request_reason = ui->reason_ed->text();
    if(request_reason.isEmpty()){
        request_reason = "";
    }
    jsonObj["send_reason"] = request_reason;

    auto back_name = ui->back_ed->text();
    if(back_name.isEmpty()){
        back_name = "";
    }

    jsonObj["send_backname"] = back_name;

    jsonObj["send_touid"] = _si->_uid;

    QJsonDocument send_apply(jsonObj);
    QByteArray jsonData = send_apply.toJson(QJsonDocument::Compact);

    /* 检查是不是搜索自身
     * 如果是，即使点击添加好友也直接return
     */
    if(_si->_uid == uid){
        //这里可以做消息提示
        //TODO 消息提示
        qDebug() << "Cannot Add self";
        return;
    }

    emit TcpMgr::GetInstance()->sig_tcp_send_data(ReqId::REQ_ADD_FRIEND_REQ, jsonData);


    this->hide();
    deleteLater();

    return;
}
