#ifndef APPLYFRIEND_H
#define APPLYFRIEND_H

#include <QDialog>
#include "clicklabel.h"
#include "friendlabel.h"
#include "userdata.h"

namespace Ui {
class ApplyFriend;
}

class ApplyFriend : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriend(QWidget *parent = nullptr);
    ~ApplyFriend();
    /*测试：初始化一些label，用于测试*/
    void InitTipLbs();
    /*添加标签到展示区*/
    void AddTipLbs(ClickLabel*, QPoint cur_point, QPoint &next_point, int text_width, int text_height);
    /*事件过滤器：隐藏或显示滚动条*/
    bool eventFilter(QObject *obj, QEvent *event);
    /*将搜索的内容保存*/
    void SetSearchInfo(std::shared_ptr<SearchInfo> si);

protected:
    /*窗口拖动逻辑*/
    void mousePressEvent(QMouseEvent *event) override; // 鼠标按下事件
    void mouseMoveEvent(QMouseEvent *event) override;  // 鼠标移动事件
    void mouseReleaseEvent(QMouseEvent *event) override; // 鼠标释放事件



private:
    Ui::ApplyFriend *ui;
    /*重置标签位置，删除某个标签后，所有标签需要更新自己的位置*/
    void resetLabels();

    //已经创建好的标签
    QMap<QString, ClickLabel*> _add_labels;
    std::vector<QString> _add_label_keys;
    //新添加的标签位置，即下一个标签位置
    QPoint _label_point;

    //新添加的标签
    //输入框输入后，敲击回车，可以在输入框产生标签栏
    QMap<QString, FriendLabel*> _friend_labels;
    std::vector<QString> _friend_label_keys;
    //增加一个新标签，检查_friend_labels判重
    //更新标签位置，包括输入框和标题栏显示框
    void addLabel(QString name);

    //缓存的已存在的标签数据
    std::vector<QString> _tip_data;
    //提示位置记录
    QPoint _tip_cur_point;
    std::shared_ptr<SearchInfo> _si;

    QPoint m_dragPosition; // 用于记录鼠标拖动时的位置
    bool m_isDragging;     // 是否正在拖动窗口
public slots:
    //显示更多label标签
    void ShowMoreLabel();
    //输入label按下回车后，将标签加入展示栏，并处理其他相关操作
    void SlotLabelEnter();
    //点击关闭按钮，移除展示栏好友便签
    void SlotRemoveFriendLabel(QString);
    //通过点击tip实现增加和减少好友便签
    void SlotChangeFriendLabelByTip(QString, LabelClickState);
    //输入框文本变化显示不同提示
    void SlotLabelTextChange(const QString& text);
    //输入框输入完成
    void SlotLabelEditFinished();
    //输入标签显示提示框input_tip_wid，点击提示框内容后添加好友便签
    void SlotAddFirendLabelByClickTip(QString text);
    //处理确认回调
    void SlotApplySure();
    //处理取消回调
    void SlotApplyCancel();
};

#endif
