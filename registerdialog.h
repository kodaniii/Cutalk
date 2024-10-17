#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

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

private:
    Ui::RegisterDialog *ui;
    void showTip(bool, QString);
};

#endif // REGISTERDIALOG_H
