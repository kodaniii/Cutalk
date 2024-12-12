#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include <QDialog>
#include "defs.h"

namespace Ui {
class ResetDialog;
}

class ResetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetDialog(QWidget *parent = nullptr);
    ~ResetDialog();

private slots:
    void on_return_btn_clicked();

    void on_verify_btn_clicked();

    void slot_reset_mod_finish(ReqId, QString, StatusCodes);
    void on_sure_btn_clicked();

private:
    bool checkUserValid();
    bool checkPasswdValid();
    bool checkEmailValid();
    bool checkVerifyCodeValid();

    void showTip(bool, QString);
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);

    void initHandlers();
    Ui::ResetDialog *ui;
    QMap<TipErr, QString> _tip_errs;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
signals:
    void sig_switch_login();
};

#endif // RESETDIALOG_H
