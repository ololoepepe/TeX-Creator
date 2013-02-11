#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

class BPasswordWidget;

class QLineEdit;
class QPushButton;

#include <QDialog>

/*============================================================================
================================ RegisterDialog ==============================
============================================================================*/

class RegisterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RegisterDialog(QWidget *parent = 0);
private slots:
    void checkRegister();
    void registerMe();
private:
    QLineEdit *mledtInvite;
    QLineEdit *mledtLogin;
    BPasswordWidget *mpwdwgt;
    QPushButton *mbtnRegister;
};

#endif // REGISTERDIALOG_H
