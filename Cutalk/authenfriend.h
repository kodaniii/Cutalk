#ifndef AUTHENFRIEND_H
#define AUTHENFRIEND_H

#include <QDialog>
#include "clicklabel.h"
#include "friendlabel.h"
#include "userdata.h"

namespace Ui {
class AuthenFriend;
}

class AuthenFriend : public QDialog
{
    Q_OBJECT

public:
    explicit AuthenFriend(QWidget *parent = nullptr);
    ~AuthenFriend();

    void InitTipLbs();
    void AddTipLbs(ClickLabel*, QPoint cur_point, QPoint &next_point, int text_width, int text_height);
    bool eventFilter(QObject *obj, QEvent *event) override;
    void SetApplyInfo(std::shared_ptr<ApplyInfo> apply_info);

protected:
    /*窗口拖动逻辑*/
    void mousePressEvent(QMouseEvent *event) override; // 鼠标按下事件
    void mouseMoveEvent(QMouseEvent *event) override;  // 鼠标移动事件
    void mouseReleaseEvent(QMouseEvent *event) override; // 鼠标释放事件

private:
    void resetLabels();

    QMap<QString, ClickLabel*> _add_labels;
    std::vector<QString> _add_label_keys;
    QPoint _label_point;
    QMap<QString, FriendLabel*> _friend_labels;
    std::vector<QString> _friend_label_keys;
    void addLabel(QString name);
    std::vector<QString> _tip_data;
    QPoint _tip_cur_point;
public slots:
    void ShowMoreLabel();
    void SlotLabelEnter();
    void SlotRemoveFriendLabel(QString);
    void SlotChangeFriendLabelByTip(QString, LabelClickState);
    void SlotLabelTextChange(const QString& text);
    void SlotLabelEditFinished();
    void SlotAddFirendLabelByClickTip(QString text);
    void SlotApplySure();
    void SlotApplyCancel();

private:
    std::shared_ptr<ApplyInfo> _apply_info;
    Ui::AuthenFriend *ui;

    QPoint m_dragPosition; // 用于记录鼠标拖动时的位置
    bool m_isDragging;     // 是否正在拖动窗口
};

#endif // AUTHENFRIEND_H
