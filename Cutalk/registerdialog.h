#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "defs.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_verificationCode_get_Button_clicked();

    void on_cancel_button_clicked();

    void slot_reg_mod_finish(HttpReqId, QString, StatusCodes);

    void on_register_button_clicked();

private:
    Ui::RegisterDialog *ui;

    void initHttpHandlers();
    QMap<HttpReqId, std::function<void(const QJsonObject&)>> _handlers;

    void showTip(bool, QString);
};

#endif // REGISTERDIALOG_H
