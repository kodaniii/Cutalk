#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "defs.h"
#include <QKeyEvent>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Escape) {
            // 忽略ESC键事件，不调用QDialog::keyPressEvent(event)
            event->ignore();
        } else {
            // 对于其他按键事件，调用基类的keyPressEvent处理
            QDialog::keyPressEvent(event);
        }
    }

private slots:
    void on_verificationCode_get_Button_clicked();

    void on_cancel_button_clicked();

    void slot_reg_mod_finish(ReqId, QString, StatusCodes);

    void on_register_button_clicked();

    void on_return_logic_ui_button_clicked();

private:
    Ui::RegisterDialog *ui;

    void initHttpHandlers();
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;

    void showTip(bool, QString);

    bool isValidEmail(QString);

    void AddTipErr(TipErr te, QString str);
    void DelTipErr(TipErr te);
    QMap<TipErr, QString> tipErrs;

    void ChangeRegisterDialogPage();
    QTimer *count_down_timer;
    int count_down;

signals:
    void sig_switch_login();
};

#endif // REGISTERDIALOG_H
